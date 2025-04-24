/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QGlobalStatic>
#include <QHash>
#include <QHostAddress>
#include <QMutex>
#include <QObject>
#include <QSocketNotifier>
#include <QTimer>

struct ares_channeldata;
class ares_addrinfo;
class Socks5Connection;

#ifdef Q_OS_WIN
#  include <ws2tcpip.h>
#else
#  include <sys/socket.h>
#endif

class DNSResolver : public QObject {
  Q_OBJECT

 public:
  DNSResolver();
  ~DNSResolver();

  static DNSResolver* instance();

  /**
   * @brief Queues up a DNS Query to get Resolved.
   *
   * @param hostname - The requested Hostname
   * @param parent - The QObject to notify. Will call the
   * onHostnameResolved(QHostAddress) method when done.
   */
  void resolveAsync(const QString& hostname, QObject* parent);

  void setNameserver(const QList<QHostAddress>& addr);

 signals:
  void setupDnsSocket(qintptr sd, const QHostAddress& dest);

 private slots:
  void requestDestroyed();
  void socketAcivated(QSocketDescriptor sd, QSocketNotifier::Type type);
  void aresTimeout();

 private:
  void addressInfoCallback(QObject* ctx, int status, int timeouts,
                           struct ares_addrinfo* result);
  void shutdownAres();
  void updateTimeout();

  // C-Ares Socket callback methods.
  qintptr aresSocket(int domain, int type, int proto);
  int aresClose(qintptr sd);
  int aresConnect(qintptr sd, const struct sockaddr* sa, socklen_t socklen,
                  unsigned int flags);
  int aresSetsockopt(qintptr sd, int opt, const void* val, int vlen);
  int aresRecvfrom(qintptr sd, void* data, size_t len, int flags,
                   struct sockaddr* sa, socklen_t* socklen);
  int aresSendto(qintptr sd, const void* data, size_t len, int flags,
                 const struct sockaddr* sa, socklen_t socklen);
  int aresGetsockname(qintptr sd, struct sockaddr* sa, socklen_t* socklen);

  static unsigned int aresNametoindex(const char* ifname, void* user_data);
  static const char* aresIndextoname(unsigned int ifindex, char* buf,
                                     size_t len, void* user_data);

  QHash<QObject*, QString> m_requests;
  QMutex m_requestLock;

  QHash<int, QSocketNotifier*> m_notifiers;
  QTimer m_timer;

  ares_channeldata* m_channel = nullptr;
};
