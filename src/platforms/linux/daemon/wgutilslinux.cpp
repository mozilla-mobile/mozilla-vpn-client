/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "wgutilslinux.h"
#include "logger.h"

// Import wireguard C library for Linux
#if defined(__cplusplus)
extern "C" {
#endif
#include "../../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h"
#if defined(__cplusplus)
}
#endif
// End import wireguard

namespace {
Logger logger(LOG_LINUX, "WireguardUtilsLinux");
}

bool WireguardUtilsLinux::interfaceExists() {
  // As currentInterfaces only gets wireguard interfaces, this method
  // also confirms an interface as being a wireguard interface.
  return currentInterfaces().contains(WG_INTERFACE);
};

QStringList WireguardUtilsLinux::currentInterfaces() {
  char* deviceName;
  size_t len;
  QStringList devices;
  char* deviceNames = wg_list_device_names();
  if (!deviceNames) {
    return devices;
  }
  wg_for_each_device_name(deviceNames, deviceName, len) {
    devices.append(deviceName);
  }
  free(deviceNames);
  return devices;
}
