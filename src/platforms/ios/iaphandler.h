/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IAPHANDLER_H
#define IAPHANDLER_H

#include <QObject>

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

  void stopSubscription();

  // Called by the delegate

  void unknownProductRegistered(const QString& identifier);
  void productRegistered(void* product);
  void processCompletedTransactions(const QStringList& ids);

 signals:
  void productsRegistered();

  void subscriptionFailed();
  void subscriptionCanceled();
  void subscriptionCompleted();
  void subscriptionValidated();

 private:
  IAPHandler(QObject* parent);
  ~IAPHandler();

 private:
  enum {
    eNotRegistered,
    eRegistering,
    eRegistered,
  } m_productsRegistrationState = eNotRegistered;

  QString m_productName;

  enum State {
    eActive,
    eInactive,
  } m_subscriptionState = eInactive;

  void* m_delegate = nullptr;
  void* m_product = nullptr;
};

#endif  // IAPHANDLER_H
