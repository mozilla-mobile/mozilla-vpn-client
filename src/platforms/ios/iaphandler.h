/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IAPHANDLER_H
#define IAPHANDLER_H

#include <QObject>

class IAPHandler final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(IAPHandler)

  Q_PROPERTY(QString priceValue READ priceValue NOTIFY priceValueChanged)

 public:
  static IAPHandler* createInstance();

  static IAPHandler* instance();

  Q_INVOKABLE void subscribe();

  bool hasProductsRegistered() const {
    return m_productsRegistrationState == eRegistered;
  }

  void registerProducts(const QStringList& products);

  void startSubscription();

  const QString& priceValue() const { return m_priceValue; }

 signals:
  void productsRegistered();

  void subscriptionStarted();
  void subscriptionFailed();
  void subscriptionCanceled();
  void subscriptionCompleted();
  void alreadySubscribed();

  void priceValueChanged();

 public slots:
  void stopSubscription();

  // Called by the delegate
  void unknownProductRegistered(const QString& identifier);
  void productRegistered(void* product);
  void processCompletedTransactions(const QStringList& ids);

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
  QString m_priceValue;

  enum State {
    eActive,
    eInactive,
  } m_subscriptionState = eInactive;

  void* m_delegate = nullptr;
  void* m_product = nullptr;
};

#endif  // IAPHANDLER_H
