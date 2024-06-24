/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDUTILSWINDOWS_H
#define WIREGUARDUTILSWINDOWS_H

#include <QObject>

#include "daemon/wireguardutils.h"
#include "windowsroutemonitor.h"
#include "wireguard.h"

struct WireGuardAPI;

class WireguardUtilsWindows final : public WireguardUtils {
  Q_OBJECT

 public:
  // Creates a WireguardUtilsWindows instance, may fail due to i.e
  // wireguard-nt failing to initialize, returns nullptr in that case.
  static std::unique_ptr<WireguardUtilsWindows> create(QObject* parent);
  ~WireguardUtilsWindows();

  bool interfaceExists() override;
  QString interfaceName() override {
    return WireguardUtilsWindows::s_interfaceName();
  }
  static const QString s_interfaceName() { return "MozillaVPN"; }
  bool addInterface(const InterfaceConfig& config) override;
  bool deleteInterface() override;

  bool updatePeer(const InterfaceConfig& config) override;
  bool deletePeer(const InterfaceConfig& config) override;
  QList<PeerStatus> getPeerStatus() override;

  bool updateRoutePrefix(const IPAddress& prefix) override;
  bool deleteRoutePrefix(const IPAddress& prefix) override;

  bool addExclusionRoute(const IPAddress& prefix) override;
  bool deleteExclusionRoute(const IPAddress& prefix) override;

  bool excludeLocalNetworks(const QList<IPAddress>& addresses) override;

 signals:
  void backendFailure();

 private:
  WireguardUtilsWindows(QObject* parent,
                        std::unique_ptr<WireGuardAPI> wireguard_api);
  void buildMibForwardRow(const IPAddress& prefix, void* row);

  quint64 m_luid = 0;
  std::unique_ptr<WireGuardAPI> m_wireguard_api;
  ulong m_deviceIpv4_Handle = 0;

  WindowsRouteMonitor m_routeMonitor;
  WIREGUARD_ADAPTER_HANDLE m_adapter = NULL;
};

#endif  // WIREGUARDUTILSWINDOWS_H
