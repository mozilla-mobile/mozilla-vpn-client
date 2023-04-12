/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PRODUCTSHANDLER_H
#define PRODUCTSHANDLER_H

#include <QAbstractListModel>
#include <QList>

class QJsonValue;

class ProductsHandler : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ProductsHandler)

 public:
  enum ProductType {
    ProductMonthly,
    ProductHalfYearly,
    ProductYearly,
    ProductUnknown = -1
  };
  Q_ENUM(ProductType);
  enum ModelRoles {
    ProductIdentifierRole = Qt::UserRole + 1,
    ProductPriceRole,
    ProductMonthlyPriceRole,
    ProductCurrencyCodeRole,
    ProductTypeRole,
    ProductFeaturedRole,
    ProductSavingsRole,
    ProductTrialDaysRole,
  };

  struct Product {
    QString m_name;
    QString m_price;
    QString m_monthlyPrice;
    QString m_currencyCode;
    int m_trialDays = 0;
    // This is not exposed and it's not localized. It's used to compute the
    // saving %.
    double m_nonLocalizedMonthlyPrice = 0;
    ProductType m_type = ProductsHandler::ProductMonthly;
    bool m_featuredProduct = false;
    // This is the % compared with the montly subscription.
    uint32_t m_savings = 0;
    // Used by individual implementations to store extra pieces they need
    void* m_extra = nullptr;
  };

  static ProductsHandler* createInstance();
  static ProductsHandler* instance();

  bool hasProductsRegistered() const {
    return m_productsRegistrationState == eRegistered;
  }
  bool isRegistering() const {
    return m_productsRegistrationState == eRegistering;
  }
  Product* findProduct(const QString& productIdentifier);
  void registerProducts(const QByteArray& data);
  QList<Product> products() { return m_products; }
  static uint32_t productTypeToMonthCount(ProductType type);

  // QAbstractListModel methods
  QHash<int, QByteArray> roleNames() const override;
  int rowCount(const QModelIndex&) const override;
  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void productsRegistered();
  void productsRegistrationStopped();

 public slots:
  // Called by the native code delegate
  void unknownProductRegistered(const QString& identifier);
  void productsRegistrationCompleted();
  void stopProductsRegistration();

 protected:
  ProductsHandler(QObject* parent);
  ~ProductsHandler();

  bool m_last_userSubscriptionNeeded = true;

  enum {
    eNotRegistered,
    eRegistering,
    eRegistered,
  } m_productsRegistrationState = eNotRegistered;

  void addProduct(const QJsonValue& value);
  void computeSavings();
  void sortPlans();
  static ProductType productTypeToEnum(const QString& type);
  QList<Product> m_products;
};

#endif  // IAPHANDLER_H
