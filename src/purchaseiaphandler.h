/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PURCHASEIAPHANDLER_H
#define PURCHASEIAPHANDLER_H

#include <QObject>

#include "productshandler.h"
#include "purchasehandler.h"

class PurchaseIAPHandler : public PurchaseHandler {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(PurchaseIAPHandler)

 public:
  PurchaseIAPHandler(QObject* parent);
  virtual ~PurchaseIAPHandler();
  static PurchaseIAPHandler* instance();

  void startSubscription(const QString& productIdentifier) override;
  void startRestoreSubscription() override;
  void cancelSubscription() override{};

 protected:
  virtual void nativeStartSubscription(ProductsHandler::Product* product) = 0;
  virtual void nativeRestoreSubscription() = 0;
};

#endif  // PURCHASEIAPHANDLER_H
