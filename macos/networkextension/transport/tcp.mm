/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import "tcp.h"

#include <sys/socket.h>
#include <netinet/tcp.h>

constexpr const int STREAM_HEADER_SIZE = 2;
constexpr const int DGRAM_MAX_SIZE = 4096;
constexpr const int DGRAM_QUEUE_SIZE = 64;

@implementation TcpTransport {
  // The destination
  struct sockaddr_storage m_destaddr;

  // The sockets.
  int             m_socket;
  int             m_downstream;
  int             m_upstream;

  // Packet buffers.
  NSData*         m_txqueue[DGRAM_QUEUE_SIZE];
  uint8_t         m_txnext;
  uint8_t         m_txcount;

  uint8_t         m_rxbuf[DGRAM_MAX_SIZE];
  size_t          m_rxoff;
  size_t          m_rxlen;

  // A packet worker thread.
  NSCondition*    m_condition;
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

  // Create a TCP socket for the outbound data stream.
  m_socket = socket(dest->sa_family, SOCK_STREAM, 0);
  if (m_socket < 0) {
    return nil;
  }

  // Attempt to minimize TCP queueing.
  int enable = 1;
  setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));

  m_condition = [NSCondition new];
  m_worker = [[NSThread alloc] initWithTarget:self
                                     selector:@selector(tcpWorker:)
                                       object:nil];
  return self;
}

- (void) dealloc {
  NSLog(@"tcp dealloc");
  [self shutdown];
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

  // Instruct the tcp worker to shutdown.
  [m_worker cancel];
  [m_condition signal];
}

- (void) tcpWorker:(id)arg {
  NSLog(@"tcp worker started");

  NSThread* thread = [NSThread currentThread];
  while (true) {
    // Wait for another packet.
    [m_condition lock];
    while (!thread.cancelled && (m_txcount == 0)) {
      [m_condition wait];
    }
    if (thread.cancelled) {
      [m_condition unlock];
      break;
    }

    // Pull the next packet off the queue.
    NSData* dgram = m_txqueue[m_txnext];
    m_txqueue[m_txnext] = nil;
    m_txnext = (m_txnext + 1) % DGRAM_QUEUE_SIZE;
    m_txcount--;
    [m_condition unlock];

    // Send the datagram out the TCP socket.
    int tx = [self sendDgram:dgram];
    if (tx == 0) {
      NSLog(@"tcp socket closed");
      break;
    }
    if (tx < 0) {
      NSLog(@"tcp socket error: %s", strerror(errno));
      break;
    }
  }

  NSLog(@"tcp worker exited");
}

- (int) sendDgram:(NSData*)dgram {
  // Send the stream header.
  const uint8_t* data = (const uint8_t*)dgram.bytes;
  uint8_t header[STREAM_HEADER_SIZE] = {};
  header[0] = (dgram.length >> 8) & 0xff;
  header[1] = dgram.length & 0xff;

  // We are going to do this in one crazy loop to try and gather as much
  // of the outbound data together into single system calls. We do this
  // because want to use both TCP_NODELAY to disable nagling and MSG_DONTWAIT
  // to avoid blocking for TCP acks.
  NSUInteger offset = 0;
  NSUInteger total = dgram.length + STREAM_HEADER_SIZE;
  while (offset < total) {
    int tx;
    if (offset >= STREAM_HEADER_SIZE) {
      // Header has been sent.
      tx = send(m_socket, data + offset - STREAM_HEADER_SIZE, total - offset, MSG_DONTWAIT);
    } else {
      // Still have header bytes to send.
      struct iovec iov[2];
      iov[0].iov_base = header + offset;
      iov[0].iov_len = STREAM_HEADER_SIZE - offset;
      iov[1].iov_base = (void*)data;
      iov[1].iov_len = dgram.length;

      struct msghdr msg = {};
      msg.msg_iov = iov;
      msg.msg_iovlen = 2;
      tx = sendmsg(m_socket, &msg, MSG_DONTWAIT);
    }

    if (tx > 0) {
      offset += tx;
      continue;
    } if (tx == 0) {
      return 0;
    }

    if (errno == EAGAIN) {
      // Wait for the socket to become writeable.
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(m_socket, &fds);
      if (select(m_socket + 1, NULL, &fds, &fds, NULL) >= 0) {
        continue;
      }
    }
    if (errno == EINTR) {
      continue;
    }
    return -1;
  }

  // Success
  return total;
}

- (void)send:(NSData*)dgram {
  [m_condition lock];
  if (m_txcount < DGRAM_QUEUE_SIZE) {
    m_txqueue[(m_txnext + m_txcount) % DGRAM_QUEUE_SIZE] = [NSData dataWithData:dgram];
    m_txcount++;
    [m_condition signal];
  }
  [m_condition unlock];
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
