/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "firewallutilsmacos.h"

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("FirewallUtilsMacos");
}  // namespace

FirewallUtilsMacos::FirewallUtilsMacos(Daemon* daemon) : FirewallUtils(daemon) {
  MZ_COUNT_CTOR(FirewallUtilsMacos);
  logger.debug() << "FirewallUtilsMacos created.";
  m_daemon = daemon;
}

FirewallUtilsMacos::~FirewallUtilsMacos() {
  MZ_COUNT_DTOR(FirewallUtilsMacos);
  logger.debug() << "FirewallUtilsMacos destroyed.";
}
