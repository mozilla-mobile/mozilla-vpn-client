/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WGUTILSMACOS_H
#define WGUTILSMACOS_H

#include <QObject>
#include <QSocketNotifier>

#include "daemon/wireguardutils.h"
#include "macosroutemonitor.h"

class WgSessionMacos;

class WgUtilsMacos final : public WireguardUtils {
  Q_OBJECT

 public:
  WgUtilsMacos(QObject* parent);
  ~WgUtilsMacos();

  bool interfaceExists() override { return m_tunfd >= 0; }
  QString interfaceName() override { return m_ifname; }
  bool addInterface(const InterfaceConfig& config) override;
  bool deleteInterface() override;

  bool updatePeer(const InterfaceConfig& config) override;
  bool deletePeer(const InterfaceConfig& config) override;
  QList<PeerStatus> getPeerStatus() override;

  bool updateRoutePrefix(const IPAddress& prefix) override;
  bool deleteRoutePrefix(const IPAddress& prefix) override;
  bool excludeLocalNetworks(const QList<IPAddress>& lanAddressRanges) override;

 signals:
  void backendFailure();

 private:
  void mtuUpdate(int proto, const QHostAddress& gateway, int ifindex, int mtu);

  int m_tunfd = -1;
  QString m_ifname;
  MacosRouteMonitor* m_rtmonitor = nullptr;

  // bridging sockets for multihop
  int m_mhopEntrySocket = -1;
  int m_mhopExitSocket = -1;

  QHash<QString, WgSessionMacos*> m_peers;
};

#endif  // WGUTILSMACOS_H
