/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <arpa/inet.h>

#include <QString>

#include "daemon.h"
#include "logger.h"
#include "loghandler.h"
#include "wgutils.h"
#include "wgquickprocess.h"  // source of WG_INTERFACE

// Import wireguard C library
#if defined(__cplusplus)
extern "C" {
#endif
#include "../../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h"
#if defined(__cplusplus)
}
#endif
// End import wireguard

namespace {
Logger logger(LOG_MAIN, "WireguardUtils");
}

/*
 * HELPERS
 */

char* iface_name() { return QString(WG_INTERFACE).toLocal8Bit().data(); }

/*
 * END HELPERS
 */

/*
 * PUBLIC METHODS
 */

// static
bool WireguardUtils::interfaceExists() {
  // Also confirms it is wireguard.
  return WireguardUtils::currentInterfaces().contains(iface_name());
};

// static
QStringList WireguardUtils::currentInterfaces() {
  char *deviceNames, *deviceName;
  size_t len;
  QStringList devices;
  deviceNames = wg_list_device_names();
  wg_for_each_device_name(deviceNames, deviceName, len) {
    devices.append(deviceName);
  }
  free(deviceNames);
  return devices;
}

/*
 * END PUBLIC METHODS
 */
