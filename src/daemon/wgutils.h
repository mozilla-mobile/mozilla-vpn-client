/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDUTILS_H
#define WIREGUARDUTILS_H

class QStringList;

class WireguardUtils final {
 public:
  // checks
  static bool interfaceExists();
  static QStringList currentInterfaces();
};

#endif  // WIREGUARDUTILS_H