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
struct FWPM_SUBLAYER_CHANGE0_;

class WindowsBypass final : public QObject {
  Q_OBJECT

 public:
  explicit WindowsBypass(Socks5* proxy);
  ~WindowsBypass();

 private:
  static QString win32strerror(unsigned long code);
  quint64 getVpnLuid() const;
  void updateTable(QVector<struct _MIB_IPFORWARD_ROW2>& table, int family);
  const struct _MIB_IPFORWARD_ROW2* lookupRoute(const QHostAddress& dest) const;
  static void setupFirewall(void* context,
                            const struct FWPM_SUBLAYER_CHANGE0_* change);
  void restrictProxyPort(quint16 port);

 private slots:
  void outgoingConnection(QAbstractSocket* s, const QHostAddress& dest);
  void interfaceChanged(quint64 luid);
  void refreshRoutes(int family);
  void refreshAddresses();

 private:
  void* m_addrChangeHandle = nullptr;
  void* m_netChangeHandle = nullptr;
  void* m_routeChangeHandle = nullptr;
  void* m_fwEngineHandle = nullptr;
  void* m_fwChangeHandle = nullptr;

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
