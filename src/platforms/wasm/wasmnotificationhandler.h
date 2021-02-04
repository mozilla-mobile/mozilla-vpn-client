/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WASMNOTIFICATIONHANDLER_H
#define WASMNOTIFICATIONHANDLER_H

#include "notificationhandler.h"

class WasmNotificationHandler final : public NotificationHandler {
 public:
  WasmNotificationHandler(QObject* parent);
  ~WasmNotificationHandler();

 protected:
  void notify(const QString& title, const QString& message,
              int timerSec) override;
};

#endif  // WASMNOTIFICATIONHANDLER_H
