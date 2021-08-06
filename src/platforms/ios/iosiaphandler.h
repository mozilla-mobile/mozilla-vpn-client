/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOSIAPHANDLER_H
#define IOSIAPHANDLER_H

#include "platforms/iaphandler.h"

#include <QList>

class QJsonValue;

class IOSIAPHandler final : public IAPHandler {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(IOSIAPHandler)

 public:
  static IOSIAPHandler* createInstance();

  static IOSIAPHandler* instance();

  void registerProducts(const QByteArray& data);

  void startSubscription(const QString& productIdentifier);
  Q_INVOKABLE void subscribe(const QString& productIdentifier);

  // QAbstractListModel methods
  QHash<int, QByteArray> roleNames() const override;
  int rowCount(const QModelIndex&) const override;
  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void productsRegistered();

  void subscriptionStarted(const QString& productIdentifier);
  void subscriptionFailed();
  void subscriptionCanceled();
  void subscriptionCompleted();
  void alreadySubscribed();

 public slots:
  void stopSubscription();

  // Called by the delegate
  void unknownProductRegistered(const QString& identifier);
  void productRegistered(void* product);
  void productsRegistrationCompleted();
  void processCompletedTransactions(const QStringList& ids);

 private:
  explicit IOSIAPHandler(QObject* parent);
  ~IOSIAPHandler();

  void addProduct(const QJsonValue& value);
  void computeSavings();

  static ProductType productTypeToEnum(const QString& type);

  static uint32_t productTypeToMonthCount(ProductType type);

 private:

  Product* findProduct(const QString& productIdentifier);

  QList<Product> m_products;

  void* m_delegate = nullptr;
};

#endif  // IOSIAPHANDLER_H
