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
  QStringList currentInterfaces() override;

  uint32_t getFirewallMark();
};

#endif  // WIREGUARDUTILSLINUX_H
