/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import "tcp.h"

#include <sys/socket.h>
#include <netinet/tcp.h>

constexpr const int STREAM_HEADER_SIZE = 2;
constexpr const int DGRAM_MAX_SIZE = 4096;

struct txbuffer {
    uint8_t data[STREAM_HEADER_SIZE + DGRAM_MAX_SIZE];
    size_t  length;
    size_t  offset;
};

@implementation TcpTransport {
  // The destination
  struct sockaddr_storage m_destaddr;

  // The sockets.
  int             m_socket;
  int             m_sockerr;
  int             m_downstream;
  int             m_upstream;

  // Packet buffers.
  struct txbuffer m_txbuf;
  uint8_t         m_rxbuf[DGRAM_MAX_SIZE];
  size_t          m_rxoff;
  size_t          m_rxlen;

  // A packet worker thread.
  NSThread*       m_worker;
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
    close(m_upstream);
    close(m_downstream);
    return nil;
  }

  // Attempt to minimize TCP queueing.
  int enable = 1;
  setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));

  m_sockerr = 0;
  m_worker = [[NSThread alloc] initWithTarget:self
                                     selector:@selector(tcpWorker:)
                                       object:nil];
  return self;
}

- (void) dealloc {
  NSLog(@"tcp dealloc");
  close(m_upstream);
  close(m_downstream);
  close(m_socket);
#if !__has_feature(objc_arc)
  [super dealloc];
#endif
}

- (int) connect {
  int err = connect(m_socket, (struct sockaddr*)&m_destaddr, m_destaddr.ss_len);
  if (err < 0) {
    return err;
  }

  // Start the socket worker
  [m_worker start];
  return 0;
}

- (void)shutdown {
  shutdown(m_socket, SHUT_RDWR);
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

    if (m_txbuf.length == 0) {
      FD_SET(m_downstream, &rfds);
    } else {
      FD_SET(m_socket, &wfds);
    }
    int result = select(nfds, &rfds, &wfds, NULL, NULL);
    if (result < 0) {
      continue;
    }

    // Fetch more packets, if possible.
    if (m_txbuf.length == 0) {
      // Try to read another packet from the downstream socket.
      int rx = recv(m_downstream, m_txbuf.data + STREAM_HEADER_SIZE, DGRAM_MAX_SIZE, MSG_DONTWAIT);
      if (rx < 0) {
        if (errno == EAGAIN) continue;
        if (errno == EINTR) continue;
        NSLog(@"tcp dgram error: %s", strerror(errno));
        break;
      }
      if (rx == 0) {
        NSLog(@"tcp dgram closed");
        break;
      }
      m_txbuf.data[0] = (rx >> 8) & 0xff;
      m_txbuf.data[1] = rx & 0xff;
      m_txbuf.length = rx + STREAM_HEADER_SIZE;
      m_txbuf.offset = 0;
    }

    // Send any queued packets.
    int tx = [self sendOutbound];
    if (tx < 0) {
      if (errno == EAGAIN) continue;
      if (errno == EINTR) continue;
      NSLog(@"tcp socket error: %s", strerror(errno));
      break;
    }
    if (tx == 0) {
      NSLog(@"tcp socket closed");
      break;
    }
  }
}

- (int) sendOutbound {
  if (m_txbuf.length == 0) {
    errno = EAGAIN;
    return -1;
  }

  while (m_txbuf.offset < m_txbuf.length) {
    // The buffer is in use, there is data to be sent.
    int tx = send(m_socket, m_txbuf.data + m_txbuf.offset,
                  m_txbuf.length - m_txbuf.offset, MSG_DONTWAIT);
    if (tx <= 0) {
      return tx;
    }
    m_txbuf.offset += tx;
  }

  int result = m_txbuf.length;
  m_txbuf.length = 0;
  m_txbuf.offset = 0;
  return result;
}

- (void)send:(NSData*)dgram {
  send(m_upstream, dgram.bytes, dgram.length, MSG_DONTWAIT);
}

- (int)recv:(NSMutableData*)dgram {
  if (m_rxlen == 0) {
    // Peek the length and start a new packet reception.
    uint8_t header[STREAM_HEADER_SIZE];
    int rx = recv(m_socket, header, sizeof(header), MSG_PEEK);
    if (rx <= 0) {
      return rx;
    }
    if (rx < sizeof(header)) {
      errno = EAGAIN;
      return -1;
    }

    // Read and discard the header, it would be an error to fail after
    // a successful peek though.
    int drop = recv(m_socket, header, sizeof(header), MSG_DONTWAIT);
    if (drop <= 0) {
      return drop;
    }
    if (drop < sizeof(header)) {
      errno = EPROTO;
      return -1;
    }
    m_rxoff = 0;
    m_rxlen = (header[0] << 8) + header[1];
    if (!m_rxlen) {
      // Empty packet.
      errno = EAGAIN;
      return -1;
    }
  }

  // Receive the packet body.
  while (m_rxoff < m_rxlen) {
    size_t remainder = m_rxlen - m_rxoff;
    uint8_t* ptr = m_rxbuf;

    if (m_rxlen <= sizeof(m_rxbuf)) {
      ptr += m_rxoff;
    } else if (remainder > sizeof(m_rxbuf)) {
      remainder = sizeof(m_rxbuf);
    }

    int rx = recv(m_socket, ptr, remainder, 0);
    if (rx <= 0) {
      return rx;
    }
    m_rxoff += rx;
  }

  if (m_rxlen > sizeof(m_rxbuf)) {
    // Packet overflow - discard the buffer contents.
    m_rxoff = 0;
    m_rxlen = 0;
    errno = EAGAIN;
    return -1;
  }

  // Return the packet to the caller.
  memcpy(dgram.mutableBytes, m_rxbuf, m_rxlen);
  dgram.length = m_rxlen;
  m_rxlen = 0;
  m_rxoff = 0;
  return dgram.length;
}

@end
