
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IAPHANDLER_H
#define IAPHANDLER_H

#include <QAbstractListModel>

class IAPHandler : public QAbstractListModel {
  Q_OBJECT

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
  };

  bool hasProductsRegistered() const {
    return m_productsRegistrationState == eRegistered;
  }

protected: // Can some of this be private once I move the methods around?
  explicit IAPHandler(QObject* parent);
  virtual ~IAPHandler();

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
    ProductType m_type = IAPHandler::ProductMonthly;
    bool m_featuredProduct = false;
    // This is the % compared with the montly subscription.
    uint32_t m_savings = 0;
    void* m_productNS = nullptr;
  };

};

#endif  // IAPHANDLER_H