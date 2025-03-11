/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QGlobalStatic>
#include <QHash>
#include <QHostAddress>
#include <QMutex>
#include <QObject>

struct ares_channeldata;
class ares_addrinfo;
class Socks5Connection;

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

 private slots:
  void requestDestroyed();

 private:
  void addressInfoCallback(QObject* ctx, int status, int timeouts,
                           struct ares_addrinfo* result);
  void shutdownAres();

  QHash<QObject*,QString> m_requests;
  QMutex m_requestLock;

  ares_channeldata* mChannel = nullptr;
};
