/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDHELPER_H
#define WIREGUARDHELPER_H

#include <QStringList>

class WireguardHelper final {
 public:
  static bool interface_exists();
  static QStringList current_wireguard_devices();
  static bool add_if();
  static bool del_dev();
};

#endif  // WIREGUARDHELPER_H
