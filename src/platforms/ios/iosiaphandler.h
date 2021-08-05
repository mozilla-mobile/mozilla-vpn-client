/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IAPHANDLER_H
#define IAPHANDLER_H

#include <QAbstractListModel>
#include <QList>

class QJsonValue;

class IOSIAPHandler final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(IOSIAPHandler)

 public:
  enum ProductType {
    ProductMonthly,
    ProductHalfYearly,
    ProductYearly,
    ProductUnknown = -1
  };
  Q_ENUM(ProductType);

  static IOSIAPHandler* createInstance();

  static IOSIAPHandler* instance();

  enum ModelRoles {
    ProductIdentifierRole = Qt::UserRole + 1,
    ProductPriceRole,
    ProductMonthlyPriceRole,
    ProductTypeRole,
    ProductFeaturedRole,
    ProductSavingsRole,
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
  void productsRegistrationCompleted();
  void processCompletedTransactions(const QStringList& ids);

 private:
  IOSIAPHandler(QObject* parent);
  ~IOSIAPHandler();

  void addProduct(const QJsonValue& value);
  void computeSavings();

  static ProductType productTypeToEnum(const QString& type);

  static uint32_t productTypeToMonthCount(ProductType type);

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
    QString m_monthlyPrice;
    // This is not exposed and it's not localized. It's used to compute the
    // saving %.
    double m_nonLocalizedMonthlyPrice = 0;
    ProductType m_type = IOSIAPHandler::ProductMonthly;
    bool m_featuredProduct = false;
    // This is the % compared with the montly subscription.
    uint32_t m_savings = 0;
    void* m_productNS = nullptr;
  };

  Product* findProduct(const QString& productIdentifier);

  QList<Product> m_products;

  void* m_delegate = nullptr;
};

#endif  // IAPHANDLER_H
