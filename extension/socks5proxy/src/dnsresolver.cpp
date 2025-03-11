
#include "dnsresolver.h"

#include <ares.h>

#include <QCoreApplication>
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

  struct ares_options options;
  int optmask = 0;
  memset(&options, 0, sizeof(options));
  optmask |= ARES_OPT_EVENT_THREAD;
  options.evsys = ARES_EVSYS_DEFAULT;
  if (ares_init_options(&mChannel, &options, optmask) != ARES_SUCCESS) {
    printf("c-ares initialization issue\n");
  }
}

DNSResolver::~DNSResolver() { ares_destroy(mChannel); }

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
  ares_getaddrinfo(mChannel, name.c_str(), NULL, &hints, callback, parent);
}

void DNSResolver::setNameserver(const QList<QHostAddress>& dnsList) {
  QString buffer{};

  foreach (const QHostAddress& dns, qAsConst(dnsList)) {
    if (dns.isNull()) {
      continue;
    }
    buffer.append(dns.toString());
    buffer.append(",");
  }
  auto serverString = buffer.toLocal8Bit();
  qDebug() << "Setting nameservers:" << serverString;
  int result = ares_set_servers_csv(mChannel, serverString.constData());
  Q_ASSERT(result == ARES_SUCCESS);
}
