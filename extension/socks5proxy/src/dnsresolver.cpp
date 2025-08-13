
#include "dnsresolver.h"

#include <ares.h>

#ifdef Q_OS_WIN
#  include <netioapi.h>
#  include <winsock2.h>
#else
#  include <fcntl.h>
#  include <net/if.h>
#  include <unistd.h>
#endif

#include <QCoreApplication>
#include <QDebug>
#include <QMutexLocker>
#include <QObject>

#include "socks5connection.h"

Q_GLOBAL_STATIC(DNSResolver, dnsResolver);
DNSResolver* DNSResolver::instance() { return dnsResolver; }

DNSResolver::DNSResolver() {
  static int s_ares_init = false;
  if (!s_ares_init) {
    ares_library_init(ARES_LIB_INIT_ALL);
    QObject::connect(qApp, &QCoreApplication::aboutToQuit,
                     []() { ares_library_cleanup(); });
    s_ares_init = true;
  }

  if (ares_init_options(&m_channel, nullptr, 0) != ARES_SUCCESS) {
    printf("c-ares initialization issue\n");
  }

  // Wrap some callbacks to make them member method calls.
  static auto cbSocket = [](int domain, int type, int proto,
                            void* ctx) -> ares_socket_t {
    return static_cast<DNSResolver*>(ctx)->aresSocket(domain, type, proto);
  };
  static auto cbClose = [](ares_socket_t sd, void* ctx) -> int {
    return static_cast<DNSResolver*>(ctx)->aresClose(sd);
  };
  static auto cbConnect = [](ares_socket_t sd, const struct sockaddr* sa,
                             ares_socklen_t socklen, unsigned int flags,
                             void* ctx) -> int {
    return static_cast<DNSResolver*>(ctx)->aresConnect(sd, sa, socklen, flags);
  };
  static auto cbSetsockopt = [](ares_socket_t sd, ares_socket_opt_t opt,
                                const void* val, ares_socklen_t vlen,
                                void* ctx) -> int {
    DNSResolver* dns = static_cast<DNSResolver*>(ctx);
    return dns->aresSetsockopt(sd, opt, val, vlen);
  };
  static auto cbRecvfrom = [](ares_socket_t sd, void* data, size_t dlen,
                              int flags, struct sockaddr* sa,
                              ares_socklen_t* slen, void* ctx) -> ares_ssize_t {
    DNSResolver* dns = static_cast<DNSResolver*>(ctx);
    return dns->aresRecvfrom(sd, data, dlen, flags, sa, slen);
  };
  static auto cbSendto = [](ares_socket_t sd, const void* data, size_t dlen,
                            int flags, const struct sockaddr* sa,
                            ares_socklen_t slen, void* ctx) -> ares_ssize_t {
    DNSResolver* dns = static_cast<DNSResolver*>(ctx);
    return dns->aresSendto(sd, data, dlen, flags, sa, slen);
  };
  static auto cbGetsockname = [](ares_socket_t sd, struct sockaddr* sa,
                                 socklen_t* socklen, void* ctx) -> int {
    DNSResolver* dns = static_cast<DNSResolver*>(ctx);
    return dns->aresGetsockname(sd, sa, socklen);
  };

  // Register callback methods for the sockets.
  static struct ares_socket_functions_ex s_functions = {
      .version = 1,
      .flags = ARES_SOCKFUNC_FLAG_NONBLOCKING,
      .asocket = cbSocket,
      .aclose = cbClose,
      .asetsockopt = cbSetsockopt,
      .aconnect = cbConnect,
      .arecvfrom = cbRecvfrom,
      .asendto = cbSendto,
      .agetsockname = cbGetsockname,
      .abind = nullptr,
      .aif_nametoindex = aresNametoindex,
      .aif_indextoname = aresIndextoname,
  };
  ares_set_socket_functions_ex(m_channel, &s_functions, this);

  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, this, &DNSResolver::aresTimeout);
}

DNSResolver::~DNSResolver() { ares_destroy(m_channel); }

/* Callback that is called when DNS query is finished */
void DNSResolver::addressInfoCallback(QObject* ctx, int status, int timeouts,
                                      struct ares_addrinfo* result) {
  QMutexLocker locker(&m_requestLock);
  if (!m_requests.contains(ctx)) {
    qDebug() << "Connection destroyed before resolution completed";
    return;
  }

  // This should be our Socks5Connection
  switch (status) {
    case ARES_SUCCESS:
      break;
    case ARES_ENOTIMP:
      qDebug() << "The ares library does not know how to find addresses of "
                  "type family. ";
      QMetaObject::invokeMethod(ctx, "onHostnameNotFound",
                                Qt::QueuedConnection);
      return;
    case ARES_ENOTFOUND:
      qDebug() << "The name was not found.";
      QMetaObject::invokeMethod(ctx, "onHostnameNotFound",
                                Qt::QueuedConnection);
      return;
    case ARES_ENOMEM:
      qDebug() << "Memory was exhausted.";
      return;
    case ARES_ESERVICE:
      qDebug() << "The textual service name provided could not be dereferenced "
                  "into a port. ";
      return;
    case ARES_EDESTRUCTION:
      qDebug() << "The name service channel channel is being destroyed; the "
                  "query will not be completed. ";
      return;
    default:
      qDebug() << "Name resolution error:" << ares_strerror(status);
      return;
  }

  Q_ASSERT(status == ARES_SUCCESS);

  auto guard = qScopeGuard([&]() { ares_freeaddrinfo(result); });
  if (!result) {
    return;
  }
  for (auto node = result->nodes; node != NULL; node = node->ai_next) {
    if (node->ai_family != AF_INET && node->ai_family != AF_INET6) {
      continue;
    }
    QHostAddress target{node->ai_addr};
    QMetaObject::invokeMethod(ctx, "onHostnameResolved", Qt::QueuedConnection,
                              Q_ARG(QHostAddress, target));
  }
}

void DNSResolver::requestDestroyed() {
  QMutexLocker locker(&m_requestLock);
  m_requests.remove(QObject::sender());
}

void DNSResolver::resolveAsync(const QString& hostname, QObject* parent) {
  // Store the requesting connections in a hash map.
  // This allows us to detect when connections are destroyed, and prevents
  // use-after-free bugs if the resolution completes after the connection
  // is freed.
  QObject::connect(parent, &QObject::destroyed, this,
                   &DNSResolver::requestDestroyed);
  m_requestLock.lock();
  m_requests.insert(parent, hostname);
  m_requestLock.unlock();

  auto callback = [](void* arg, int status, int timeouts,
                     struct ares_addrinfo* results) {
    QObject* ctx = static_cast<QObject*>(arg);
    auto instance = DNSResolver::instance();
    instance->addressInfoCallback(ctx, status, timeouts, results);
  };

  auto name = hostname.toStdString();
  struct ares_addrinfo_hints hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = ARES_AI_CANONNAME;
  ares_getaddrinfo(m_channel, name.c_str(), NULL, &hints, callback, parent);
  updateTimeout();
}

void DNSResolver::setNameserver(const QList<QHostAddress>& dnsList) {
  QString buffer{};

  for (const QHostAddress& dns : dnsList) {
    if (dns.isNull()) {
      continue;
    }
    buffer.append(dns.toString());
    buffer.append(",");
  }
  auto serverString = buffer.toLocal8Bit();
  qDebug() << "Setting nameservers:" << serverString;
  int result = ares_set_servers_csv(m_channel, serverString.constData());
  Q_ASSERT(result == ARES_SUCCESS);
}

void DNSResolver::socketAcivated(QSocketDescriptor sd,
                                 QSocketNotifier::Type type) {
  ares_fd_events_t ev = {.fd = static_cast<ares_socket_t>(sd)};
  if (type == QSocketNotifier::Read) {
    ev.events = ARES_FD_EVENT_READ;
  }
  if (type == QSocketNotifier::Write) {
    ev.events = ARES_FD_EVENT_WRITE;
  }
  ares_process_fds(m_channel, &ev, 1, ARES_PROCESS_FLAG_NONE);
  updateTimeout();
}

void DNSResolver::updateTimeout() {
  struct timeval next;
  struct timeval maxtv = {.tv_sec = 60, .tv_usec = 0};
  struct timeval* tv = ares_timeout(m_channel, &maxtv, &next);
  int msec = tv->tv_sec * 1000 + tv->tv_usec / 1000;
  m_timer.start(qMax(1, msec));
}

void DNSResolver::aresTimeout() {
  ares_process_fds(m_channel, nullptr, 0, ARES_PROCESS_FLAG_NONE);
  updateTimeout();
}

qintptr DNSResolver::aresSocket(int domain, int type, int proto) {
  qintptr sd = socket(domain, type, proto);
  if (sd < 0) {
    return sd;
  }

  // Configure the socket for non-blocking operation.
#ifdef Q_OS_WIN
  u_long mode = 1;
  ioctlsocket(sd, FIONBIO, &mode);
#else
  int flags = fcntl(sd, F_GETFL, 0);
  if (flags == -1) {
    close(sd);
    return -1;
  }
  fcntl(sd, F_SETFL, flags | O_NONBLOCK);
#endif

  // Create a socket notifier to drive the socket on reception
  QSocketNotifier* n = new QSocketNotifier(sd, QSocketNotifier::Read, this);
  connect(n, &QSocketNotifier::activated, this, &DNSResolver::socketAcivated);
  m_notifiers[sd] = n;

  return sd;
}

int DNSResolver::aresClose(qintptr sd) {
  QSocketNotifier* n = m_notifiers.take(sd);
  if (n) {
    delete n;
  }
#ifdef Q_OS_WIN
  return closesocket(sd);
#else
  return close(sd);
#endif
}

int DNSResolver::aresConnect(qintptr sd, const struct sockaddr* sa,
                             ares_socklen_t salen, unsigned int flags) {
  Q_UNUSED(flags);
  emit setupDnsSocket(sd, QHostAddress(sa));
  return ::connect(sd, sa, salen);
}

int DNSResolver::aresSetsockopt(qintptr sd, int opt, const void* val,
                                int vlen) {
  const char* vptr = static_cast<const char*>(val);
  switch (opt) {
    case ARES_SOCKET_OPT_SENDBUF_SIZE:
      return setsockopt(sd, SOL_SOCKET, SO_SNDBUF, vptr, vlen);
    case ARES_SOCKET_OPT_RECVBUF_SIZE:
      return setsockopt(sd, SOL_SOCKET, SO_RCVBUF, vptr, vlen);
    default:
      // Not Implemented.
      errno = ENOPROTOOPT;
      return -1;
  }
}

int DNSResolver::aresRecvfrom(qintptr sd, void* data, size_t len, int flags,
                              struct sockaddr* sa, socklen_t* socklen) {
  return recvfrom(sd, static_cast<char*>(data), len, flags, sa, socklen);
}

int DNSResolver::aresSendto(qintptr sd, const void* data, size_t len, int flags,
                            const struct sockaddr* sa, socklen_t socklen) {
  return sendto(sd, static_cast<const char*>(data), len, flags, sa, socklen);
}

int DNSResolver::aresGetsockname(qintptr sd, struct sockaddr* sa,
                                 socklen_t* socklen) {
  return getsockname(sd, sa, socklen);
}

unsigned int DNSResolver::aresNametoindex(const char* ifname, void* user_data) {
  Q_UNUSED(user_data);
  return if_nametoindex(ifname);
}

const char* DNSResolver::aresIndextoname(unsigned int ifindex, char* buf,
                                         size_t len, void* user_data) {
  if (len < IF_NAMESIZE) {
    return nullptr;
  }
  return if_indextoname(ifindex, buf);
}
