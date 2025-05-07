/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FIREWALLUTILS_H
#define FIREWALLUTILS_H

#include <QObject>

#include "daemon.h"

class FirewallUtils : public QObject {
  Q_OBJECT

 public:
  explicit FirewallUtils(Daemon* daemon) : QObject(daemon){};
  virtual ~FirewallUtils() = default;

  // Returns true if split tunnelling is supported.
  virtual bool splitTunnelSupported() const { return false; }
};

#endif  // FIREWALLUTILS_H
