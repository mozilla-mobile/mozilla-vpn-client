/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PURCHASEIAPHANDLER_H
#define PURCHASEIAPHANDLER_H

#include "purchasehandler.h";

#include <QObject>

class PurchaseIAPHandler : public PurchaseHandler {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(PurchaseIAPHandler)

 public:
  static PurchaseIAPHandler* createInstance();
  static PurchaseIAPHandler* instance();

  // Returns the latest SKU the started to Subcribe.
  // Is empty if the user already had a subscription or never started the
  // subscription flow.
  const QString& currentSKU() const { return m_currentSKU; }

  Q_INVOKABLE void subscribe(const QString& productIdentifier);
  Q_INVOKABLE void restore();

  void startSubscription(const QString& productIdentifier);
  void startRestoreSubscription();

  // The nativeRegisterProducts method is currently here (not in
  // productshandler) for simplicity of the native implementation.
  virtual void nativeRegisterProducts() = 0;

 signals:
  void restoreSubscriptionStarted();
  void alreadySubscribed();
  void billingNotAvailable();
  void subscriptionNotValidated();

 protected:
  PurchaseIAPHandler(QObject* parent);
  ~PurchaseIAPHandler();

  virtual void nativeStartSubscription(ProductsHandler::Product* product) = 0;
  virtual void nativeRestoreSubscription() = 0;

  QString m_currentSKU;
};

#endif  // PURCHASEIAPHANDLER_H
