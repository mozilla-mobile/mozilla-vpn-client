/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PURCHASEHANDLER_H
#define PURCHASEHANDLER_H

#include <QObject>

class PurchaseHandler : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(PurchaseHandler)

 public:
  static PurchaseHandler* createInstance();
  static PurchaseHandler* instance();

  Q_INVOKABLE void subscribe(const QString& productIdentifier);
  Q_INVOKABLE void restore();
  Q_INVOKABLE virtual void cancelSubscription() = 0;
  virtual void startSubscription(const QString& productIdentifier) = 0;
  virtual void startRestoreSubscription() = 0;

  const QString& currentSKU() const { return m_currentSKU; }

  // The nativeRegisterProducts method is currently here (not in
  // ProductsHandler) for simplicity of the native implementation.
  // TODO - Clean this up and properly separate native products implementations.
  virtual void nativeRegisterProducts() = 0;

 signals:
  // Not all sub-classes will use all these signals.
  void subscriptionStarted(const QString& productIdentifier);
  void subscriptionFailed();
  void subscriptionCanceled();
  void subscriptionCompleted();
  void restoreSubscriptionStarted();
  void alreadySubscribed();
  void billingNotAvailable();
  void subscriptionNotValidated();

 public slots:
  void stopSubscription();

 protected:
  PurchaseHandler(QObject* parent);
  virtual ~PurchaseHandler();

  enum State {
    eActive,
    eInactive,
  } m_subscriptionState = eInactive;

  QString m_currentSKU;
};

#endif  // PURCHASEHANDLER_H
