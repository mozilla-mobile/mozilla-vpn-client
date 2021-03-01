/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDUTILSLINUX_H
#define WIREGUARDUTILSLINUX_H

#include "daemon/wgutils.h"
#include <QStringList>

class WireguardUtilsLinux final : public WireguardUtils {
 public:
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
  wg_peer* buildPeerForDevice(struct wg_device* device,
                              const InterfaceConfig& conf);
};

#endif  // WIREGUARDUTILSLINUX_H
