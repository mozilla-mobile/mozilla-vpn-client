/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WASMSTARTER_H
#define WASMSTARTER_H

#include <QObject>

class WasmStarter final {
  Q_DISABLE_COPY_MOVE(WasmStarter)

 public:
  static void initialize();

  static int s_authLoadingTimeoutMSec;
};

#endif  // WASMSTARTER_H
