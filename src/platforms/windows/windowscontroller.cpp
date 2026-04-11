/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowscontroller.h"

#include "constants.h"
#include "localsocketcontroller.h"
#include "platforms/windows/daemon/windowssplittunnel.h"

WindowsController::WindowsController() :
    LocalSocketController(Constants::WINDOWS_DAEMON_PATH) {}

bool WindowsController::splitTunnelSupported() const {
  return !WindowsSplitTunnel::detectConflict();
}
