/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "iaphandler.h"
#include "constants.h"
#include "inspector/inspectorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QScopeGuard>

#ifdef MVPN_IOS
#  include "platforms/ios/iosiaphandler.h"
#elif MVPN_ANDROID
#  include "platforms/android/androidiaphandler.h"
#else
#  include "platforms/dummy/dummyiaphandler.h"
#endif

namespace {
Logger logger(LOG_IAP, "IAPHandler");
IAPHandler* s_instance = nullptr;
}  // namespace

// static
IAPHandler* IAPHandler::createInstance() {
  Q_ASSERT(!s_instance);
#ifdef MVPN_IOS
  new IOSIAPHandler(qApp);
#elif MVPN_ANDROID
  new AndroidIAPHandler(qApp);
#else
  new DummyIAPHandler(qApp);
#endif
  Q_ASSERT(s_instance);
  return instance();
}

// static
IAPHandler* IAPHandler::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

IAPHandler::IAPHandler(QObject* parent) : QAbstractListModel(parent) {
  MVPN_COUNT_CTOR(IAPHandler);

  Q_ASSERT(!s_instance);
  s_instance = this;
}

IAPHandler::~IAPHandler() {
  MVPN_COUNT_DTOR(IAPHandler);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

void IAPHandler::registerProducts(const QByteArray& data) {
  logger.debug() << "Maybe register products";

  Q_ASSERT(m_productsRegistrationState == eRegistered ||
           m_productsRegistrationState == eNotRegistered);

  auto guard = qScopeGuard([&] { emit productsRegistered(); });

  if (m_productsRegistrationState == eRegistered) {
    return;
  }

  Q_ASSERT(m_products.isEmpty());

  QJsonDocument json = QJsonDocument::fromJson(data);
  if (!json.isObject()) {
    logger.debug() << "Object expected";
    return;
  }

  QJsonObject obj = json.object();
  if (!obj.contains("products")) {
    logger.debug() << "products entry expected";
    return;
  }

  QJsonArray products = obj["products"].toArray();
  if (products.isEmpty()) {
    logger.error() << "No products found";
    return;
  }

  m_productsRegistrationState = eRegistering;

  for (auto p : products) {
    addProduct(p);
  }

  if (m_products.isEmpty()) {
    logger.error() << "No pending products (nothing has been registered). "
                      "Unable to recover from "
                      "this scenario.";
    return;
  }

  nativeRegisterProducts();

  logger.debug() << "Waiting for the products registration";

  guard.dismiss();
}

IAPHandler::Product* IAPHandler::findProduct(const QString& productIdentifier) {
  for (Product& p : m_products) {
    if (p.m_name == productIdentifier) {
      return &p;
    }
  }
  return nullptr;
}

void IAPHandler::addProduct(const QJsonValue& value) {
  if (!value.isObject()) {
    logger.debug() << "Object expected for the single product";
    return;
  }

  QJsonObject obj = value.toObject();

  // The products endpoint returns products for all platforms
  // only continue if the product is for the current platform
  if (obj["platform"].toString().toLower() !=
      QString(Constants::PLATFORM_NAME).toLower()) {
    return;
  }

  Product product;
  product.m_name = obj["id"].toString();
  product.m_type = productTypeToEnum(obj["type"].toString());
  product.m_featuredProduct = obj["featured_product"].toBool();

  if (product.m_type == ProductUnknown) {
    logger.error() << "Unknown product type:" << obj["type"].toString();
    return;
  }
  m_products.append(product);
}

void IAPHandler::startSubscription(const QString& productIdentifier) {
  Q_ASSERT(m_productsRegistrationState == eRegistered);

  Product* product = findProduct(productIdentifier);
  Q_ASSERT(product);

  if (m_subscriptionState != eInactive) {
    logger.warning() << "No multiple IAP!";
    return;
  }
  m_subscriptionState = eActive;
  logger.debug() << "Starting the subscription";
  nativeStartSubscription(product);
}

void IAPHandler::startRestoreSubscription() {
  logger.debug() << "Starting the restore of the subscription";

#ifdef MVPN_IOS
  nativeRestoreSubscription();
#else
  logger.error() << "Restore not implemented!";
  emit subscriptionFailed();
#endif
}

void IAPHandler::stopSubscription() {
  logger.debug() << "Stop subscription";
  m_subscriptionState = eInactive;
}

void IAPHandler::unknownProductRegistered(const QString& identifier) {
  Q_ASSERT(m_productsRegistrationState == eRegistering);

  logger.error() << "Product registration failed:" << identifier;

  // Let's remove the unregistered product.
  QList<Product>::iterator i = m_products.begin();
  while (i != m_products.end()) {
    if (i->m_name == identifier) {
      i = m_products.erase(i);
      break;
    }
    ++i;
  }
}

void IAPHandler::productsRegistrationCompleted() {
  logger.debug() << "All the products has been registered";
  beginResetModel();
  computeSavings();
  sortPlans();
  m_productsRegistrationState = eRegistered;
  endResetModel();
  emit productsRegistered();
}

void IAPHandler::stopProductsRegistration() {
  logger.debug() << "Stop products registration";
  beginResetModel();
  m_products.clear();
  m_productsRegistrationState = eNotRegistered;
  endResetModel();
  emit productsRegistrationStopped();
}

void IAPHandler::subscribe(const QString& productIdentifier) {
  logger.debug() << "Subscription required";
  emit subscriptionStarted(productIdentifier);
}

void IAPHandler::restore() {
  logger.debug() << "Restore purchase";
  emit restoreSubscriptionStarted();
}

QHash<int, QByteArray> IAPHandler::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[ProductIdentifierRole] = "productIdentifier";
  roles[ProductPriceRole] = "productPrice";
  roles[ProductMonthlyPriceRole] = "productMonthlyPrice";
  roles[ProductTrialDaysRole] = "productTrialDays";
  roles[ProductTypeRole] = "productType";
  roles[ProductFeaturedRole] = "productFeatured";
  roles[ProductSavingsRole] = "productSavings";
  return roles;
}

int IAPHandler::rowCount(const QModelIndex&) const {
  if (m_productsRegistrationState != eRegistered) {
    return 0;
  }

  return m_products.count();
}

QVariant IAPHandler::data(const QModelIndex& index, int role) const {
  if (m_productsRegistrationState != eRegistered || !index.isValid()) {
    return QVariant();
  }

  switch (role) {
    case ProductIdentifierRole:
      return QVariant(m_products.at(index.row()).m_name);

    case ProductPriceRole:
      return QVariant(m_products.at(index.row()).m_price);

    case ProductMonthlyPriceRole:
      return QVariant(m_products.at(index.row()).m_monthlyPrice);

    case ProductTypeRole:
      return QVariant(m_products.at(index.row()).m_type);

    case ProductFeaturedRole:
      return QVariant(m_products.at(index.row()).m_featuredProduct);

    case ProductSavingsRole:
      return QVariant(m_products.at(index.row()).m_savings);

    case ProductTrialDaysRole:
      if (Feature::get(Feature::Feature_freeTrial)->isSupported()) {
        if ((m_products.at(index.row()).m_type == ProductYearly) &&
            InspectorHandler::mockFreeTrial()) {
          return QVariant(7);
        }
        return QVariant(m_products.at(index.row()).m_trialDays);
      }
      return QVariant(0);

    default:
      return QVariant();
  }
}

void IAPHandler::computeSavings() {
  double monthlyPrice = 0;
  // Let's find the price for the monthly payment.
  for (const Product& product : m_products) {
    if (product.m_type == ProductMonthly) {
      monthlyPrice = product.m_nonLocalizedMonthlyPrice;
      break;
    }
  }

  if (monthlyPrice == 0) {
    logger.error() << "No monthly payment found";
    return;
  }

  // Compute the savings for all the other types.
  for (Product& product : m_products) {
    if (product.m_type == ProductMonthly) continue;
    int savings =
        qRound(100.00 -
               ((product.m_nonLocalizedMonthlyPrice * 100.00) / monthlyPrice));
    if (savings < 0 || savings > 100) continue;

    product.m_savings = (int)savings;

    logger.debug() << "Saving" << product.m_savings << "for" << product.m_name;
  }
}

// static
IAPHandler::ProductType IAPHandler::productTypeToEnum(const QString& type) {
  if (type == "yearly") return ProductYearly;
  if (type == "half-yearly") return ProductHalfYearly;
  if (type == "monthly") return ProductMonthly;
  return ProductUnknown;
}

// static
uint32_t IAPHandler::productTypeToMonthCount(ProductType type) {
  switch (type) {
    case ProductYearly:
      return 12;
    case ProductHalfYearly:
      return 6;
    case ProductMonthly:
      return 1;
    default:
      Q_ASSERT(false);
      return 1;
  }
}

void IAPHandler::sortPlans() {
  std::sort(m_products.begin(), m_products.end(),
            [](const Product& a, const Product& b) {
              return a.m_trialDays > b.m_trialDays;
            });
}
