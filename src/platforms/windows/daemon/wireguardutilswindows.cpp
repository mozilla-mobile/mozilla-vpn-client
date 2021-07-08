/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wireguardutilswindows.h"
#include "leakdetector.h"
#include "logger.h"
#include "platforms/windows/windowscommons.h"
#include "windowsdaemon.h"

#include <QScopeGuard>

#include <Windows.h>

namespace {
Logger logger(LOG_WINDOWS, "WireguardUtilsWindows");
};

WireguardUtilsWindows::WireguardUtilsWindows(QObject* parent)
    : WireguardUtils(parent) {
  MVPN_COUNT_CTOR(WireguardUtilsWindows);
  logger.log() << "WireguardUtilsWindows created.";
}

WireguardUtilsWindows::~WireguardUtilsWindows() {
  MVPN_COUNT_DTOR(WireguardUtilsWindows);
  logger.log() << "WireguardUtilsWindows destroyed.";
}

bool WireguardUtilsWindows::interfaceExists() {
  SC_HANDLE scm;
  SC_HANDLE service;

  auto guard = qScopeGuard([&] {
    if (service) {
      CloseServiceHandle(service);
    }

    if (scm) {
      CloseServiceHandle(scm);
    }
  });

  scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
  if (!scm) {
    WindowsCommons::windowsLog("Failed to open SCManager");
    return false;
  }

  // Let's see if we have to delete a previous instance.
  service = OpenService(scm, TUNNEL_SERVICE_NAME, SERVICE_ALL_ACCESS);
  if (!service) {
    return false;
  }

  SERVICE_STATUS status;
  if (!QueryServiceStatus(service, &status)) {
    return false;
  }

  return (status.dwCurrentState == SERVICE_RUNNING);
}

WireguardUtils::peerBytes WireguardUtilsWindows::getThroughputForInterface() {
  peerBytes pb = {0,0};
  return pb;
}

// Dummy implementations for now
bool WireguardUtilsWindows::addInterface() { return true; }
bool WireguardUtilsWindows::configureInterface(const InterfaceConfig& config) {
  Q_UNUSED(config);
  return true;
}

bool WireguardUtilsWindows::deleteInterface() { return true; }

bool WireguardUtilsWindows::addRoutePrefix(const IPAddressRange& prefix) {
  Q_UNUSED(prefix);
  return true;
}

