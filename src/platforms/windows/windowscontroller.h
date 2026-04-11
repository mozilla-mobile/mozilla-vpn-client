/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSCONTROLLER_H
#define WINDOWSCONTROLLER_H

#include "localsocketcontroller.h"

class WindowsController final : public LocalSocketController {
  Q_DISABLE_COPY_MOVE(WindowsController)

 public:
  WindowsController();

  bool splitTunnelSupported() const override;
};

#endif // WINDOWSCONTROLLER_H
