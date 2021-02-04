/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/wasm/wasmnotificationhandler.h"
#include "leakdetector.h"
#include "platforms/wasm/wasmwindowcontroller.h"

WasmNotificationHandler::WasmNotificationHandler(QObject* parent)
    : NotificationHandler(parent) {
  MVPN_COUNT_CTOR(WasmNotificationHandler);
}

WasmNotificationHandler::~WasmNotificationHandler() {
  MVPN_COUNT_DTOR(WasmNotificationHandler);
}

void WasmNotificationHandler::notify(const QString& title,
                                     const QString& message, int timerSec) {
  Q_UNUSED(timerSec);
  WasmWindowController::instance()->notification(title, message);
}
