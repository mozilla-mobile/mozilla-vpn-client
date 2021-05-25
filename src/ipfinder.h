/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IPFINDER_H
#define IPFINDER_H

#include <QObject>

class QHostAddress;
class QHostInfo;

class IPFinder final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(IPFinder)

 public:
  explicit IPFinder(QObject* parent);
  ~IPFinder();

  void start();

 signals:
  void completed(const QString& ipv4, const QString& ipv6,
                 const QString& country);

 private slots:
  void dnsLookupCompleted(const QHostInfo& hostInfo);

 private:
  void createRequest(const QHostAddress& address, bool ipv6);
  void completeLookup();

 private:
  struct IPLookup {
    QString m_country;
    QString m_ipAddress;
    bool m_ipv6;
  };

  QList<IPLookup> m_results;
  uint32_t m_requestCount = 0;
  int m_lookupId = -1;
};

#endif  // IPFINDER_H
