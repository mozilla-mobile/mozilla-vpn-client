/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FIREWALLUTILSMACOS_H
#define FIREWALLUTILSMACOS_H

#include <QObject>

#include "daemon/firewallutils.h"

class FirewallUtilsMacos final : public FirewallUtils {
  Q_OBJECT
 
 public:
  FirewallUtilsMacos(Daemon* daemon);
  ~FirewallUtilsMacos();

  bool splitTunnelSupported() const override;

 private:
  Daemon* m_daemon = nullptr;
  void* m_loader = nullptr;
};

#endif  // FIREWALLUTILSMACOS_H
 