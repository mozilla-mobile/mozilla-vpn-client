/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IPUTILSLINUX_H
#define IPUTILSLINUX_H

#include "daemon/iputils.h"

#include <arpa/inet.h>

class IPUtilsLinux final : public IPUtils {
 public:
  bool addInterfaceIPs(const InterfaceConfig& config) override;

 private:
  struct in6_ifreq {
    struct in6_addr addr;
    uint32_t prefixlen;
    unsigned int ifindex;
  };
  bool addIP4AddressToDevice(const InterfaceConfig& config);
  bool addIP6AddressToDevice(const InterfaceConfig& config);
};

#endif  // IPUTILSLINUX_H