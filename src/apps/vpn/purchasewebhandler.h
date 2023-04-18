/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PURCHASEWEBHANDLER_H
#define PURCHASEWEBHANDLER_H

#include "purchasehandler.h"

class PurchaseWebHandler final : public PurchaseHandler {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(PurchaseWebHandler)

 public:
  PurchaseWebHandler(QObject* parent);
  ~PurchaseWebHandler();

  void cancelSubscription() override;
  void nativeRegisterProducts() override;
  void startSubscription(const QString& productIdentifier) override;
  void startRestoreSubscription() override;

 private slots:
  void subscriptionDone(const QByteArray& json, const QString& token);
};

#endif  // PURCHASEWEBHANDLER_H
