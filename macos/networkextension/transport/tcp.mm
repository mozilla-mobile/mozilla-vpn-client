/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import "tcp.h"

#include <sys/socket.h>

constexpr const int STREAM_HEADER_SIZE = 2;
constexpr const int DGRAM_MAX_SIZE = PIPE_BUF;

struct pktbuffer {
    uint8_t data[STREAM_HEADER_SIZE + DGRAM_MAX_SIZE];
    size_t  length;
    size_t  offset;
};

@implementation TcpTransport {
  // The destination
  struct sockaddr_storage m_destaddr;

  // The sockets.
  int       m_socket;
  int       m_sockerr;
  int       m_downstream;
  int       m_upstream;

  // Packet buffers.
  struct pktbuffer  m_txbuf;
  struct pktbuffer  m_rxbuf;

  // A packet worker thread.
  NSThread*         m_worker;
}

- (id) initWithEndpoint:(nw_endpoint_t) ep {
  self = [super init];

  // Save the address for use in connect().
  const struct sockaddr* dest = nw_endpoint_get_address(ep);
  if (dest->sa_len > sizeof(m_destaddr)) {
    return nil;
  }
  memcpy(&m_destaddr, dest, dest->sa_len);

  // Create a datagram socket pair to pass wireguard packets.
  int sv[2];
  int err = socketpair(AF_UNIX, SOCK_DGRAM, 0, sv);
  if (err != 0) {
    return nil;
  }
  m_upstream = sv[0];
  m_downstream = sv[1];

  // Create a TCP socket for the outbound data stream.
  m_socket = socket(dest->sa_family, SOCK_STREAM, 0);
  if (m_socket < 0) {
    m_sockerr = errno;
    close(m_upstream);
    close(m_downstream);
    return nil;
  }

  m_sockerr = 0;
  return self;
}

- (int) connect {
  int err = connect(m_socket, (struct sockaddr*)&m_destaddr, m_destaddr.ss_len);
  if (err < 0) {
    m_sockerr = errno;
    return err;
  }

  m_worker = [[NSThread alloc] initWithTarget:self
                                     selector:@selector(tcpWorker:)
                                       object:nil];
  return 0;
}

- (void) tcpWorker:(id)arg {
  NSLog(@"tcp worker started");
  int nfds = m_downstream;
  if (m_socket > m_downstream) {
    nfds = m_socket;
  }

  NSThread* thread = [NSThread currentThread];
  while (!thread.cancelled) {
    fd_set rfds;
    fd_set wfds;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);

    FD_SET(m_socket, &rfds);
    if (m_txbuf.length == 0) {
      FD_SET(m_downstream, &rfds);
    }
    int result = select(nfds, &rfds, &wfds, NULL, NULL);
    if (result < 0) {
      continue;
    }

    if (FD_ISSET(m_socket, &rfds)) {
      [self handleInbound];
    }
    [self handleOutbound];
  }
}

- (void) handleInbound {
  while (true) {
    // Get the next packet length.
    if (m_rxbuf.offset < STREAM_HEADER_SIZE) {
      int len = recv(m_socket, m_rxbuf.data + m_rxbuf.offset, STREAM_HEADER_SIZE - m_rxbuf.offset, MSG_DONTWAIT);
      if (len == 0) {
        m_sockerr = ECONNRESET;
        return;
      }
      if (len < 0) {
        break;
      }
      m_rxbuf.length = (m_rxbuf.data[0] << 8) + m_rxbuf.data[1] + STREAM_HEADER_SIZE;
      if (m_rxbuf.length <= STREAM_HEADER_SIZE) {
        m_rxbuf.length = 0;
        m_rxbuf.offset = 0;
        continue;
      }
    }

    size_t remainder = m_rxbuf.length - m_rxbuf.offset;

    // If the packet is too big, read and discard it.
    if (m_rxbuf.length > sizeof(m_rxbuf.data)) {
      if (remainder > sizeof(m_rxbuf.data)) {
        remainder = sizeof(m_rxbuf.data);
      }
      int len = recv(m_socket, m_rxbuf.data, remainder, MSG_DONTWAIT);
      if (len == 0) {
        m_sockerr = ECONNRESET;
        return;
      }
      if (len < 0) {
        break;
      }
      m_rxbuf.offset += len;
      continue;
    }

    // Receive more data from the socket.
    if (m_rxbuf.offset < m_rxbuf.length) {
      int len = recv(m_socket, m_rxbuf.data + m_rxbuf.offset, remainder, MSG_DONTWAIT);
      if (len == 0) {
        m_sockerr = ECONNRESET;
        return;
      }
      if (len < 0) {
        break;
      }
      m_rxbuf.offset += len;
    }

    // If the packet is complete, send it to the UNIX datagram socket.
    if (m_rxbuf.offset >= m_rxbuf.length) {
      send(m_downstream, m_rxbuf.data + STREAM_HEADER_SIZE,
           m_rxbuf.length - STREAM_HEADER_SIZE, MSG_DONTWAIT);
      m_rxbuf.length = 0;
      m_rxbuf.offset = 0;
    }
  }

  // Save fatal errors for reporting.
  if (errno == EAGAIN) return;
  if (errno == EINTR) return;
  m_sockerr = errno;
}

- (void) handleOutbound {
  while (true) {
    // The buffer is in use, there is data to be sent.
    if (m_txbuf.length) {
      int len = send(m_socket, m_txbuf.data + m_txbuf.offset,
                     m_txbuf.length - m_txbuf.offset, MSG_DONTWAIT);
      if (len > 0) {
        // Some data was sent.
        m_txbuf.offset += len;
        if (m_txbuf.offset < m_txbuf.length) {
          return;
        }
        m_txbuf.offset = 0;
        m_txbuf.length = 0;
      } else if (len == 0) {
        m_sockerr = ECONNRESET;
        return;
      } else {
        if (errno == EAGAIN) return;
        if (errno == EINTR) return;
        m_sockerr = errno;
        return;
      }
    }

    // Try to read another packet from the downstream socket.
    int len = recv(m_downstream, m_txbuf.data + STREAM_HEADER_SIZE, DGRAM_MAX_SIZE, MSG_DONTWAIT);
    if (len <= 0) {
      return;
    }
    m_txbuf.data[0] = (len >> 8) & 0xff;
    m_txbuf.data[1] = len & 0xff;
    m_txbuf.length = len + STREAM_HEADER_SIZE;
    m_txbuf.offset = 0;
  }
}

- (void)send:(NSData*)dgram {
  send(m_upstream, dgram.bytes, dgram.length, MSG_DONTWAIT);
}

- (int)recv:(NSMutableData*)dgram {
  // Propagate TCP socket errors to the caller.
  if (m_sockerr != 0) {
    errno = m_sockerr;
    return -1;
  }

  int rx = read(m_upstream, dgram.mutableBytes, dgram.length);
  if (rx > 0) {
    dgram.length = rx;
  }
  return rx;
}

@end
