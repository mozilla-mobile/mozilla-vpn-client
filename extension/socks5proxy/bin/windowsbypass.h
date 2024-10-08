/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSBYPASS_H
#define WINDOWSBYPASS_H

#include <QHash>
#include <QHostAddress>
#include <QObject>
#include <QVector>

class Socks5;
class QAbstractSocket;
class QHostAddress;

struct _MIB_IPFORWARD_ROW2;

class WindowsBypass final : public QObject {
  Q_OBJECT

 public:
  explicit WindowsBypass(Socks5* proxy);
  ~WindowsBypass();

 private:
  static QString win32strerror(unsigned long code);
  void updateTable(QVector<struct _MIB_IPFORWARD_ROW2>& table, int family);
  quint64 getVpnLuid() const;
  const struct _MIB_IPFORWARD_ROW2* lookupRoute(const QHostAddress& dest) const;

 private slots:
  void outgoingConnection(QAbstractSocket* s, const QHostAddress& dest);
  void refreshRoutes(int family);
  void refreshIfMetrics();
  void refreshAddresses();

 private:
  void* m_addrChangeHandle = nullptr;
  void* m_netChangeHandle = nullptr;
  void* m_routeChangeHandle = nullptr;

  struct InterfaceData {
    unsigned long metric;
    QHostAddress ipv4addr;
    QHostAddress ipv6addr;
  };

  QHash<quint64, InterfaceData> m_interfaceData;
  QVector<struct _MIB_IPFORWARD_ROW2> m_routeTableIpv4;
  QVector<struct _MIB_IPFORWARD_ROW2> m_routeTableIpv6;
};

#endif  // WINDOWSBYPASS_H
