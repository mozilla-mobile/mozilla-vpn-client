/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IAPHANDLER_H
#define IAPHANDLER_H

#include <QAbstractListModel>
#include <QList>

class QJsonValue;

class IAPHandler final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(IAPHandler)

 public:
  enum ProductType { ProductMonthly, ProductYearly, ProductUnknown = -1 };

  static IAPHandler* createInstance();

  static IAPHandler* instance();

  enum ModelRoles {
    ProductIdentifierRole = Qt::UserRole + 1,
    ProductPriceRole,
    ProductTypeRole,
  };
  Q_INVOKABLE void subscribe(const QString& productIdentifier);

  bool hasProductsRegistered() const {
    return m_productsRegistrationState == eRegistered;
  }

  void registerProducts(const QByteArray& data);

  void startSubscription(const QString& productIdentifier);

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
  void processCompletedTransactions(const QStringList& ids);

 private:
  IAPHandler(QObject* parent);
  ~IAPHandler();

  void registerProduct(const QJsonValue& value);
  void sortProductsAndCompleteRegistration();

  static ProductType productTypeToEnum(const QString& type);

 private:
  enum {
    eNotRegistered,
    eRegistering,
    eRegistered,
  } m_productsRegistrationState = eNotRegistered;

  enum State {
    eActive,
    eInactive,
  } m_subscriptionState = eInactive;

  struct Product {
    QString m_name;
    QString m_price;
    ProductType m_type = IAPHandler::ProductMonthly;
    void* m_product = nullptr;
  };

  static bool sortProductsCallback(const IAPHandler::Product& a,
                                   const IAPHandler::Product& b);

  QList<Product> m_registeredProducts;
  QHash<QString, Product> m_pendingProducts;

  void* m_delegate = nullptr;
};

#endif  // IAPHANDLER_H
