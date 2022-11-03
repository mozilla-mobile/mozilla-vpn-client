/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PURCHASEIAPHANDLER_H
#define PURCHASEIAPHANDLER_H

#include "purchasehandler.h"

#include <QAbstractListModel>
#include <QList>

class QJsonValue;

class PurchaseIAPHandler : public PurchaseHandler {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(PurchaseIAPHandler)

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
    ProductTypeRole,
    ProductFeaturedRole,
    ProductSavingsRole,
    ProductTrialDaysRole,
  };

  bool hasProductsRegistered() const {
    return m_productsRegistrationState == eRegistered;
  }

  // Returns the latest SKU the started to Subcribe.
  // Is empty if the user already had a subscription or never started the
  // subscription flow.
  const QString& currentSKU() const { return m_currentSKU; }

  Q_INVOKABLE void subscribe(
      const QString& productIdentifier = QString()) override;
  Q_INVOKABLE void restore();

  void registerProducts(const QByteArray& data);
  void startSubscription(const QString& productIdentifier);
  void startRestoreSubscription();

  // QAbstractListModel methods
  QHash<int, QByteArray> roleNames() const override;
  int rowCount(const QModelIndex&) const override;
  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void alreadySubscribed();
  void billingNotAvailable();
  void productsRegistered();
  void productsRegistrationStopped();
  void subscriptionFailed();
  void subscriptionNotValidated();
  void subscriptionCanceled();
  void subscriptionCompleted();
  void restoreSubscriptionStarted();

 public slots:
  void stopSubscription();
  // Called by the native code delegate
  void unknownProductRegistered(const QString& identifier);
  void productsRegistrationCompleted();
  void stopProductsRegistration();

 public:
  explicit PurchaseIAPHandler(QObject* parent);
  ~PurchaseIAPHandler();

 protected:
  struct Product {
    QString m_name;
    QString m_price;
    QString m_monthlyPrice;
    int m_trialDays = 0;
    // This is not exposed and it's not localized. It's used to compute the
    // saving %.
    double m_nonLocalizedMonthlyPrice = 0;
    ProductType m_type = PurchaseIAPHandler::ProductMonthly;
    bool m_featuredProduct = false;
    // This is the % compared with the montly subscription.
    uint32_t m_savings = 0;
    // Used by individual implementations to store extra pieces they need
    void* m_extra = nullptr;
  };

  virtual void nativeRegisterProducts() = 0;
  virtual void nativeStartSubscription(Product* product) = 0;
  virtual void nativeRestoreSubscription() = 0;

  enum {
    eNotRegistered,
    eRegistering,
    eRegistered,
  } m_productsRegistrationState = eNotRegistered;

  void addProduct(const QJsonValue& value);
  void computeSavings();
  void sortPlans();
  static ProductType productTypeToEnum(const QString& type);
  static uint32_t productTypeToMonthCount(ProductType type);
  Product* findProduct(const QString& productIdentifier);
  QList<Product> m_products;
  QString m_currentSKU;
};

#endif  // PURCHASEIAPHANDLER_H
