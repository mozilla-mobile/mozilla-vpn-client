/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDUTILSMACOS_H
#define WIREGUARDUTILSMACOS_H

#include <QSocketNotifier>
#include <QObject>

#include "daemon/wireguardutils.h"
#include "macosroutemonitor.h"

class WgSessionMacos;

class WireguardUtilsMacos final : public WireguardUtils {
  Q_OBJECT

 public:
  WireguardUtilsMacos(QObject* parent);
  ~WireguardUtilsMacos();

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
  void tunOutput(const QByteArray& packet);

 private slots:
  void tunActivated(QSocketDescriptor sd, QSocketNotifier::Type type);

 private:
  void tunInput(const QByteArray& packet);

  int m_tunfd = -1;
  QString m_ifname;
  QSocketNotifier* m_tunNotifier = nullptr;
  MacosRouteMonitor* m_rtmonitor = nullptr;

  QHash<QString,WgSessionMacos*> m_peers;
};

#endif  // WIREGUARDUTILSMACOS_H
