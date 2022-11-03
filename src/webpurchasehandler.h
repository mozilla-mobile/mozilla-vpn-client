/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WebPurchaseHandler_H
#define WebPurchaseHandler_H

#include "purchasehandler.h"

class WebPurchaseHandler final : public PurchaseHandler {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(WebPurchaseHandler)

 public:
  explicit WebPurchaseHandler(QObject* parent);
  ~WebPurchaseHandler();

 protected:
  void nativeRegisterProducts() override;
  void nativeStartSubscription(Product* product) override;
  void nativeRestoreSubscription() override;
  void startSubscription(const QString& productIdentifier);
};

#endif  // WebPurchaseHandler_H
