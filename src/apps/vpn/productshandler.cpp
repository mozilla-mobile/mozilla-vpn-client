/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "productshandler.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QScopeGuard>

#include "constants.h"
#include "feature.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/user.h"
#include "mozillavpn.h"
#include "notificationhandler.h"
#include "purchasehandler.h"
#include "tasks/products/taskproducts.h"
#include "taskscheduler.h"

namespace {
Logger logger("ProductsHandler");
ProductsHandler* s_instance = nullptr;
}  // namespace

// static
ProductsHandler* ProductsHandler::createInstance() {
  Q_ASSERT(!s_instance);
  new ProductsHandler(qApp);
  Q_ASSERT(s_instance);
  return instance();
}

// static
ProductsHandler* ProductsHandler::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

ProductsHandler::ProductsHandler(QObject* parent) : QAbstractListModel(parent) {
  MZ_COUNT_CTOR(ProductsHandler);
  Q_ASSERT(!s_instance);
  s_instance = this;

  // On iAP make sure the Products are loaded in time.
  // If we Move into any State adjecent to iAP - load the
  // products if we don't have that already.
  connect(MozillaVPN::instance(), &MozillaVPN::stateChanged, this, [this]() {
    auto state = MozillaVPN::instance()->state();
    if ((state == App::StateSubscriptionNeeded ||
         state == App::StateSubscriptionInProgress ||
         state == App::StateAuthenticating) &&
        !this->hasProductsRegistered()) {
      TaskScheduler::scheduleTask(new TaskProducts());
    }
  });

  m_last_userSubscriptionNeeded =
      MozillaVPN::instance()->user()->subscriptionNeeded();

  connect(MozillaVPN::instance()->user(), &User::changed, [this] {
    auto userSubscriptionNeeded =
        MozillaVPN::instance()->user()->subscriptionNeeded();
    if (!m_last_userSubscriptionNeeded && userSubscriptionNeeded) {
      NotificationHandler::instance()->subscriptionNotFoundNotification();
    }
    m_last_userSubscriptionNeeded = userSubscriptionNeeded;
  });
}

ProductsHandler::~ProductsHandler() {
  MZ_COUNT_DTOR(ProductsHandler);
  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

void ProductsHandler::registerProducts(const QByteArray& data) {
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

  PurchaseHandler::instance()->nativeRegisterProducts();

  logger.debug() << "Waiting for the products registration";

  guard.dismiss();
}

ProductsHandler::Product* ProductsHandler::findProduct(
    const QString& productIdentifier) {
  for (Product& p : m_products) {
    if (p.m_name == productIdentifier) {
      return &p;
    }
  }
  return nullptr;
}

void ProductsHandler::addProduct(const QJsonValue& value) {
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

void ProductsHandler::unknownProductRegistered(const QString& identifier) {
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

void ProductsHandler::productsRegistrationCompleted() {
  logger.debug() << "All the products has been registered";
  beginResetModel();
  computeSavings();
  sortPlans();
  m_productsRegistrationState = eRegistered;
  endResetModel();
  emit productsRegistered();
}

void ProductsHandler::stopProductsRegistration() {
  logger.debug() << "Stop products registration";
  beginResetModel();
  m_products.clear();
  m_productsRegistrationState = eNotRegistered;
  endResetModel();
  emit productsRegistrationStopped();
}

QHash<int, QByteArray> ProductsHandler::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[ProductIdentifierRole] = "productIdentifier";
  roles[ProductPriceRole] = "productPrice";
  roles[ProductMonthlyPriceRole] = "productMonthlyPrice";
  roles[ProductCurrencyCodeRole] = "productCurrencyCode";
  roles[ProductTrialDaysRole] = "productTrialDays";
  roles[ProductTypeRole] = "productType";
  roles[ProductFeaturedRole] = "productFeatured";
  roles[ProductSavingsRole] = "productSavings";
  return roles;
}

int ProductsHandler::rowCount(const QModelIndex&) const {
  if (m_productsRegistrationState != eRegistered) {
    return 0;
  }

  return static_cast<int>(m_products.count());
}

QVariant ProductsHandler::data(const QModelIndex& index, int role) const {
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

    case ProductCurrencyCodeRole:
      return QVariant(m_products.at(index.row()).m_currencyCode);

    case ProductTypeRole:
      return QVariant(m_products.at(index.row()).m_type);

    case ProductFeaturedRole:
      return QVariant(m_products.at(index.row()).m_featuredProduct);

    case ProductSavingsRole:
      return QVariant(m_products.at(index.row()).m_savings);

    case ProductTrialDaysRole:
      if (Feature::get(Feature::Feature_freeTrial)->isSupported()) {
        if ((m_products.at(index.row()).m_type == ProductYearly) &&
            MozillaVPN::mockFreeTrial()) {
          return QVariant(7);
        }
        return QVariant(m_products.at(index.row()).m_trialDays);
      }
      return QVariant(0);

    default:
      return QVariant();
  }
}

void ProductsHandler::computeSavings() {
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
ProductsHandler::ProductType ProductsHandler::productTypeToEnum(
    const QString& type) {
  if (type == "yearly") return ProductYearly;
  if (type == "half-yearly") return ProductHalfYearly;
  if (type == "monthly") return ProductMonthly;
  return ProductUnknown;
}

// static
uint32_t ProductsHandler::productTypeToMonthCount(ProductType type) {
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

void ProductsHandler::sortPlans() {
  std::sort(m_products.begin(), m_products.end(),
            [](const Product& a, const Product& b) {
              return a.m_trialDays > b.m_trialDays;
            });
}
