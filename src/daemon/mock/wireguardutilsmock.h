/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDUTILSMOCK_H
#define WIREGUARDUTILSMOCK_H

#include <QMap>
#include <QObject>

#include "daemon/wireguardutils.h"

class WireguardUtilsMock final : public WireguardUtils {
  Q_OBJECT

 public:
  WireguardUtilsMock(QObject* parent);
  ~WireguardUtilsMock();

  bool interfaceExists() override { return true; }
  QString interfaceName() override { return WG_INTERFACE; }
  bool addInterface(const InterfaceConfig& config) override;
  bool deleteInterface() override;

  bool updatePeer(const InterfaceConfig& config) override;
  bool deletePeer(const InterfaceConfig& config) override;
  QList<PeerStatus> getPeerStatus() override;

  bool updateRoutePrefix(const IPAddress& prefix) override;
  bool deleteRoutePrefix(const IPAddress& prefix) override;

  bool addExclusionRoute(const IPAddress& prefix) override;
  bool deleteExclusionRoute(const IPAddress& prefix) override;

 signals:
  void backendFailure();

 private:
  // Keep a list of peers and when we added them.
  QMap<QString, qint64> m_handshakes;
};

#endif  // WIREGUARDUTILSMOCK_H
