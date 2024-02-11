/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionchecker.h"

#include "logger.h"

#if defined(MZ_WINDOWS)
#  include "platforms/windows/windowsconnectionchecker.h"
#endif

namespace {
Logger logger("ConnectionChecker");
}

// static
ConnectionChecker* ConnectionChecker::instance() {
#if defined(MZ_WINDOWS)
  static WindowsConnectionChecker s_instance;
  return &s_instance;
#else
  static ConnectionChecker s_instance;
  return &s_instance;
#endif
}

bool ConnectionChecker::checkConnectionAndReport() {
  return checkInternetConnectionAndReport() && checkFirewallAndReport() &&
         checkRoutingTableAndReport();
}

bool ConnectionChecker::checkInternetConnectionAndReport() { return true; }

bool ConnectionChecker::checkFirewallAndReport() { return true; }

bool ConnectionChecker::checkRoutingTableAndReport() { return true; }
