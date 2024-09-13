/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOSIAPHANDLER_H
#define IOSIAPHANDLER_H

#include "Mozilla-Swift.h"
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
  void processCompletedTransactions(const QStringList& ids,
                                    const QString transactionIdentifier);
  void noSubscriptionFoundError();

 protected:
  void nativeStartSubscription(ProductsHandler::Product* product) override;
  void nativeRestoreSubscription() override;

 private:
  void* m_delegate = nullptr;
  int discountToDays(void* discount);
  // This is a void (and cast to InAppPurchaseHandler as needed) as the
  // InAppPurchaseHandler needs to store Product, which is only available in iOS
  // 15+. So the entire class must be marked as only available in iOS 15. But we
  // can't mark this variable as only available in iOS 15+ here. Hence, we have
  // this variable as a `void`, and cast it to InAppPurchaseHandler as needed.
  void* swiftIAPHandler;
};

#endif  // IOSIAPHANDLER_H
