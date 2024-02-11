/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsconnectionchecker.h"

#include <windows.h>
#include <winrt/Windows.Networking.Connectivity.h>

#include "logger.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Networking::Connectivity;

namespace {
Logger logger("WindowsConnectionChecker");
}

bool WindowsConnectionChecker::checkInternetConnectionAndReport() {
  ConnectionProfile connectionProfile =
      NetworkInformation::GetInternetConnectionProfile();
  NetworkConnectivityLevel connectivityLevel =
      connectionProfile.GetNetworkConnectivityLevel();

  if (connectivityLevel != NetworkConnectivityLevel::InternetAccess) {
    return false;
  }

  return true;
}

bool WindowsConnectionChecker::checkFirewallAndReport() { return true; }

bool WindowsConnectionChecker::checkRoutingTableAndReport() { return true; }
