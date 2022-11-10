/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WASMIAPHANDLER_H
#define WASMIAPHANDLER_H

#include "iaphandler.h"

class WasmIAPHandler final : public IAPHandler {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(WasmIAPHandler)

 public:
  explicit WasmIAPHandler(QObject* parent);
  ~WasmIAPHandler();

 protected:
  void nativeRegisterProducts() override;
  void nativeStartSubscription(ProductsHandler::Product* product) override;
  void nativeRestoreSubscription() override;
};

#endif  // WASMIAPHANDLER_H
