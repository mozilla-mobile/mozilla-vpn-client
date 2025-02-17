
#include "dnsresolver.h"

#include <ares.h>

#include <QCoreApplication>
#include <QObject>

#include "socks5connection.h"

DNSResolver::DNSResolver() {
  std::call_once(mflag_init, &DNSResolver::initAres);
  struct ares_options options;
  int optmask = 0;
  memset(&options, 0, sizeof(options));
  optmask |= ARES_OPT_EVENT_THREAD;
  options.evsys = ARES_EVSYS_DEFAULT;
  if (ares_init_options(&mChannel, &options, optmask) != ARES_SUCCESS) {
    printf("c-ares initialization issue\n");
  }
}
std::once_flag DNSResolver::mflag_init = {};

DNSResolver::~DNSResolver() { ares_destroy(mChannel); }

void DNSResolver::initAres() {
  ares_library_init(ARES_LIB_INIT_ALL);

  // Cleanup on Shutdown
  QObject::connect(qApp, &QCoreApplication::aboutToQuit,
                   []() { ares_library_cleanup(); });
}

/* Callback that is called when DNS query is finished */
void DNSResolver::addressInfoCallback(void* arg, int status, int timeouts,
                                      struct ares_addrinfo* result) {
  // This should be our Socks5Connection
  auto ctx = static_cast<QObject*>(arg);
  switch (status) {
    case ARES_ENOTIMP:
      qDebug() << "The ares library does not know how to find addresses of "
                  "type family. ";
      QMetaObject::invokeMethod(ctx, "onHostnameNotFound",
                                Qt::QueuedConnection);
      return;
    case ARES_ENOTFOUND:
      qDebug() << " The name was not found.";
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

void DNSResolver::resolveAsync(const QString& hostname,
                               std::optional<QHostAddress> nameServer,
                               Socks5Connection* parent) {
  if (nameServer.has_value()) {
    auto server_str = nameServer->toString().toStdString();
    auto res = ares_set_servers_csv(mChannel, server_str.c_str());
    Q_ASSERT(res == ARES_SUCCESS);
  }
  auto name = hostname.toStdString();
  struct ares_addrinfo_hints hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = ARES_AI_CANONNAME;
  ares_getaddrinfo(mChannel, name.c_str(), NULL, &hints, &addressInfoCallback,
                   parent);
}
