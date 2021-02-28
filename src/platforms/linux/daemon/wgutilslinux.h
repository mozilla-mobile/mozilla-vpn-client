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
  QStringList currentInterfaces() override;
  bool removeInterfaceIfExists();
};

#endif  // WIREGUARDUTILSLINUX_H
