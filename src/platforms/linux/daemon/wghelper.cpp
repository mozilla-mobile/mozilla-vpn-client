/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <string>
#include "wghelper.h"
#include "wgquickprocess.h"  // source of WG_INTERFACE

#include <QString>
#include <QByteArray>
#include <QStringList>

#include "logger.h"
#include "loghandler.h"

// Import wireguard C library
#if defined(__cplusplus)
extern "C" {
#endif
#include "../../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h"
#if defined(__cplusplus)
}
#endif

namespace {
Logger logger(LOG_LINUX, "WireguardHelper");
}

bool WireguardHelper::interface_exists() {
  // Also confirms it is wireguard
  QStringList wg_devices = WireguardHelper::current_wireguard_devices();
  return wg_devices.contains(WG_INTERFACE);
};

QStringList WireguardHelper::current_wireguard_devices() {
  // wg_list_device_names returns first\0second\0third\0forth\0last\0\0
  char* wg_devices_raw = wg_list_device_names();
  int len = 0;
  QByteArray device = {};
  QStringList wg_devices;
  // I'm all ears on a more graceful way to handle this
  while (!(wg_devices_raw[len] == '\0' && wg_devices_raw[len - 1] == '\0')) {
    if (wg_devices_raw[len] == '\0') {
      wg_devices.append(device);
      device = {};
    } else {
      device.append(wg_devices_raw[len]);
    }
    ++len;
  }
  return wg_devices;
}