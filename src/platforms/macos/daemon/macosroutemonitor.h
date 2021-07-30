/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSROUTEMONITOR_H
#define MACOSROUTEMONITOR_H

#include "ipaddressrange.h"

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QSocketNotifier>

struct rt_msghdr;
struct sockaddr;

class MacosRouteMonitor final : public QObject {
  Q_OBJECT

 public:
  MacosRouteMonitor(const QString& ifname, QObject* parent = nullptr);
  ~MacosRouteMonitor();

  bool insertRoute(const IPAddressRange& prefix);
  bool deleteRoute(const IPAddressRange& prefix);

 private:
  void handleRtmAdd(const struct rt_msghdr* rtm, const QByteArray& payload);
  void handleRtmDelete(const struct rt_msghdr* rtm, const QByteArray& payload);
  void handleRtmChange(const struct rt_msghdr* rtm, const QByteArray& payload);
  QList<QByteArray> parseAddrList(const QByteArray& data);

 private slots:
  void rtsockReady();

 private:
  static QString addrToString(const struct sockaddr* sa);
  static QString addrToString(const QByteArray& data);

  QString m_ifname;
  int m_rtsock = -1;
  QSocketNotifier* m_notifier = nullptr;
};

#endif  // MACOSROUTEMONITOR_H
