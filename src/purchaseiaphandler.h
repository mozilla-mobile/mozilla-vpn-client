/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PURCHASEIAPHANDLER_H
#define PURCHASEIAPHANDLER_H

#include "purchasehandler.h"

#include <QObject>

class PurchaseIAPHandler : public PurchaseHandler {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(PurchaseIAPHandler)

 public:
  PurchaseIAPHandler(QObject* parent);
  ~PurchaseIAPHandler();

  void startSubscription(const QString& productIdentifier) override;
  void startRestoreSubscription() override;

 protected:
  virtual void nativeStartSubscription(ProductsHandler::Product* product) = 0;
  virtual void nativeRestoreSubscription() = 0;

  bool m_hasRestore = true;
  bool m_hasAlreadySubscribed = true;
  bool m_hasBillingMethods = true;
};

#endif  // PURCHASEIAPHANDLER_H
