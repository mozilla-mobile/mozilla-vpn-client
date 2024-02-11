/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSCONNECTIONCHECKER_H
#define WINDOWSCONNECTIONCHECKER_H

#include <QObject>

#include "connectionchecker.h"

// Singleton that checks network connection
//
class WindowsConnectionChecker final : public ConnectionChecker {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(WindowsConnectionChecker)

 public:
  WindowsConnectionChecker() = default;
  virtual ~WindowsConnectionChecker() = default;

 protected:
  virtual bool checkInternetConnectionAndReport();
  virtual bool checkFirewallAndReport();
  virtual bool checkRoutingTableAndReport();
};

#endif  // CONNECTIONCHECKER_H