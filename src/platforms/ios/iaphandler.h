/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IAPHANDLER_H
#define IAPHANDLER_H

#include <QObject>
#include <QInAppStore>

class QInAppTransaction;

class IAPHandler final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(IAPHandler)

 public:
  static IAPHandler* createInstance();

  static IAPHandler* instance();

  bool hasProductsRegistered() const {
    return m_productsRegistrationState == eRegistered;
  }

  void registerProducts(const QStringList& products);

  void startSubscription(bool restore);

 signals:
  void productsRegistered();

  void subscriptionFailed();
  void subscriptionCompleted();

 private:
  IAPHandler(QObject* parent);
  ~IAPHandler();

  void purchaseCompleted();

 private:
  QInAppStore m_appStore;

  enum {
    eNotRegistered,
    eRegistering,
    eRegistered,
  } m_productsRegistrationState = eNotRegistered;

  QString m_productName;

  bool m_started = false;
};

#endif  // IAPHANDLER_H
