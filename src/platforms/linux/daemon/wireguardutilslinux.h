/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDUTILSLINUX_H
#define WIREGUARDUTILSLINUX_H

#include "daemon/wireguardutils.h"
#include <QStringList>

class WireguardUtilsLinux final : public WireguardUtils {
 public:
  WireguardUtilsLinux(QObject* parent);
  ~WireguardUtilsLinux();
  bool interfaceExists() override;
  bool addInterface() override;
  bool configureInterface(const InterfaceConfig& config) override;
  bool deleteInterface() override;
  peerBytes getThroughputForInterface() override;

 private:
  QStringList currentInterfaces();
  bool setPeerEndpoint(struct sockaddr* peerEndpoint, const QString& address,
                       int port);
  bool setAllowedIpsOnPeer(struct wg_peer* peer,
                           QList<IPAddressRange> allowedIPAddressRanges);
  bool buildPeerForDevice(struct wg_device* device,
                          const InterfaceConfig& conf);
};

#endif  // WIREGUARDUTILSLINUX_H
