/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOSIAPHANDLER_H
#define IOSIAPHANDLER_H

#include "purchaseiaphandler.h"

class IOSIAPHandler final : public PurchaseIAPHandler {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(IOSIAPHandler)

 public:
  explicit IOSIAPHandler(QObject* parent);
  ~IOSIAPHandler();
  void nativeRegisterProducts() override;

 public slots:
  void productRegistered(void* product);
  void processCompletedTransactions(const QStringList& ids);
  void noSubscriptionFoundError();

 protected:
  void nativeStartSubscription(ProductsHandler::Product* product) override;
  void nativeRestoreSubscription() override;

 private:
  void* m_delegate = nullptr;
  int discountToDays(void* discount);
};

#endif  // IOSIAPHANDLER_H
