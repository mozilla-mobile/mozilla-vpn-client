/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDUTILSMOCK_H
#define WIREGUARDUTILSMOCK_H

#include <QHash>
#include <QObject>

#include "wireguardutils.h"

class WireguardUtilsMock final : public WireguardUtils {
  Q_OBJECT

 public:
  WireguardUtilsMock(QObject* parent);
  ~WireguardUtilsMock();

  bool interfaceExists() override { return m_config.isValid(); }
  QString interfaceName() override { return WG_INTERFACE; }
  bool addInterface(const InterfaceConfig& config) override;
  bool deleteInterface() override;

  bool updatePeer(const InterfaceConfig& config) override;
  bool deletePeer(const InterfaceConfig& config) override;
  QList<PeerStatus> getPeerStatus() override;

  bool updateRoutePrefix(const IPAddress& prefix) override;
  bool deleteRoutePrefix(const IPAddress& prefix) override;
  bool excludeLocalNetworks(const QList<IPAddress>& addresses) override;

  // Some extra stuff for testing.
  enum RouteType {
    RoutePeer = 0,
    RouteExclusion,
    RouteFailure,
  };

  void setHandshakeDelay(int msec) { m_handshakeDelayMsec = msec; }
  void setRoutePrefix(const IPAddress& prefix, RouteType type);
  const QHash<IPAddress, RouteType>& routes() const { return m_routes; }

 signals:
  void backendFailure();

 private:
  InterfaceConfig m_config;

  int m_handshakeDelayMsec = 1500;
  QHash<QString, qint64> m_handshakes;

  QHash<IPAddress, RouteType> m_routes;
};

#endif  // WIREGUARDUTILSMOCK_H
