/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDUTILSLINUX_H
#define WIREGUARDUTILSLINUX_H

#include "daemon/daemon.h"
#include "daemon/wgutils.h"
#include <QStringList>

class WireguardUtilsLinux final : public WireguardUtils {
 public:
  struct peerBytes {
    double txBytes, rxBytes;
  };
  bool interfaceExists() override;
  QStringList currentInterfaces() override;
  bool removeInterfaceIfExists();
  peerBytes getThroughputForInterface();
  bool configureInterface(const Daemon::Config& config);

 private:
  bool setPeerEndpoint(struct sockaddr* peerEndpoint, const QString& address,
                       int port);
  bool setAllowedIpsOnPeer(struct wg_peer* peer,
                           QList<IPAddressRange> allowedIPAddressRanges);
  wg_peer* buildPeerForDevice(struct wg_device* device,
                              const Daemon::Config& conf);
};

#endif  // WIREGUARDUTILSLINUX_H
