/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IPUTILSMACOS_H
#define IPUTILSMACOS_H

#include <arpa/inet.h>

#include "iputils.h"

class MacOSDaemon;

class IPUtilsMacos final : public IPUtils {
 public:
  IPUtilsMacos(MacOSDaemon* parent);
  ~IPUtilsMacos();

  bool addInterfaceIPs(const InterfaceConfig& config) override;
  bool setMTUAndUp(const InterfaceConfig& config) override;

 private:
  QString interfaceName() const;
  bool addIP4AddressToDevice(const InterfaceConfig& config);
  bool addIP6AddressToDevice(const InterfaceConfig& config);

 private:
  MacOSDaemon* m_daemon = nullptr;
};

#endif  // IPUTILSMACOS_H
