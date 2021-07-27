/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDUTILSWINDOWS_H
#define WIREGUARDUTILSWINDOWS_H

#include "daemon/wireguardutils.h"
#include "windowstunnelservice.h"

#include <QObject>

class WireguardUtilsWindows final : public WireguardUtils {
  Q_OBJECT

 public:
  WireguardUtilsWindows(QObject* parent);
  ~WireguardUtilsWindows();

  bool interfaceExists() override { return m_tunnel.isRunning(); }
  bool addInterface(const InterfaceConfig& config) override;
  bool deleteInterface() override;

  bool updatePeer(const InterfaceConfig& config) override;
  bool deletePeer(const QString& pubkey) override;
  peerStatus getPeerStatus(const QString& pubkey) override;

  bool updateRoutePrefix(const IPAddressRange& prefix, int hopindex) override;
  bool deleteRoutePrefix(const IPAddressRange& prefix, int hopindex) override;

 signals:
  void backendFailure();

 private:
  void buildMibForwardRow(const IPAddressRange& prefix, void* row);

  quint64 m_luid = 0;
  WindowsTunnelService m_tunnel;
};

#endif  // WIREGUARDUTILSWINDOWS_H
