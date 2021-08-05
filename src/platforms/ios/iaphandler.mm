/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/ios/iaphandler.h"
#include "constants.h"
#include "iosutils.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "settingsholder.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QScopeGuard>

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>

namespace {
Logger logger(LOG_IAP, "IAPHandler");

IAPHandler* s_instance = nullptr;
}  // namespace

@interface IAPHandlerDelegate
    : NSObject <SKRequestDelegate, SKProductsRequestDelegate, SKPaymentTransactionObserver> {
  IAPHandler* m_handler;
}
@end

@implementation IAPHandlerDelegate

- (id)initWithObject:(IAPHandler*)handler {
  self = [super init];
  if (self) {
    m_handler = handler;
  }
  return self;
}

- (void)productsRequest:(nonnull SKProductsRequest*)request
     didReceiveResponse:(nonnull SKProductsResponse*)response {
  logger.debug() << "Registration completed";

  if (response.invalidProductIdentifiers) {
    NSArray<NSString*>* products = response.invalidProductIdentifiers;
    logger.error() << "Registration failure" << [products count];

    for (unsigned long i = 0, count = [products count]; i < count; ++i) {
      NSString* identifier = [products objectAtIndex:i];
      QMetaObject::invokeMethod(m_handler, "unknownProductRegistered", Qt::QueuedConnection,
                                Q_ARG(QString, QString::fromNSString(identifier)));
    }
  }

  NSArray<SKProduct*>* products = response.products;
  if (products) {
    logger.debug() << "Products registered" << [products count];

    for (unsigned long i = 0, count = [products count]; i < count; ++i) {
      SKProduct* product = [[products objectAtIndex:i] retain];
      QMetaObject::invokeMethod(m_handler, "productRegistered", Qt::QueuedConnection,
                                Q_ARG(void*, product));
    }
  }

  QMetaObject::invokeMethod(m_handler, "productsRegistrationCompleted", Qt::QueuedConnection);

  [request release];
}

- (void)paymentQueue:(nonnull SKPaymentQueue*)queue
    updatedTransactions:(nonnull NSArray<SKPaymentTransaction*>*)transactions {
  logger.debug() << "payment queue:" << [transactions count];

  QStringList completedTransactionIds;
  bool failedTransactions = false;
  bool canceledTransactions = false;
  bool completedTransactions = false;

  for (SKPaymentTransaction* transaction in transactions) {
    switch (transaction.transactionState) {
      case SKPaymentTransactionStateFailed:
        logger.error() << "transaction failed";

        if (transaction.error.code == SKErrorPaymentCancelled) {
          canceledTransactions = true;
        } else {
          failedTransactions = true;
        }
        break;

      case SKPaymentTransactionStateRestored:
        [[fallthrough]];
      case SKPaymentTransactionStatePurchased: {
        QString identifier = QString::fromNSString(transaction.transactionIdentifier);
        QDateTime date = QDateTime::fromNSDate(transaction.transactionDate);
        logger.debug() << "transaction purchased - identifier: " << identifier
                       << "- date:" << date.toString();

        if (transaction.transactionState == SKPaymentTransactionStateRestored) {
          SKPaymentTransaction* originalTransaction = transaction.originalTransaction;
          if (originalTransaction) {
            QString originalIdentifier =
                QString::fromNSString(originalTransaction.transactionIdentifier);
            QDateTime originalDate = QDateTime::fromNSDate(originalTransaction.transactionDate);
            logger.debug() << "original transaction identifier: " << originalIdentifier
                           << "- date:" << originalDate.toString();
          }
        }

        completedTransactions = true;

        SettingsHolder* settingsHolder = SettingsHolder::instance();
        if (settingsHolder->hasSubscriptionTransaction(identifier)) {
          logger.warning() << "This transaction has already been processed. Let's ignore it.";
        } else {
          completedTransactionIds.append(identifier);
        }

        break;
      }
      case SKPaymentTransactionStatePurchasing:
        logger.debug() << "transaction purchasing";
        break;
      case SKPaymentTransactionStateDeferred:
        logger.debug() << "transaction deferred";
        break;
      default:
        logger.warning() << "transaction unknwon state";
        break;
    }
  }

  if (!completedTransactions && !canceledTransactions && !failedTransactions) {
    // Nothing completed, nothing restored, nothing failed. Just purchasing transactions.
    return;
  }

  if (canceledTransactions) {
    logger.debug() << "Subscription canceled";
    QMetaObject::invokeMethod(m_handler, "stopSubscription", Qt::QueuedConnection);
    QMetaObject::invokeMethod(m_handler, "subscriptionCanceled", Qt::QueuedConnection);
  } else if (failedTransactions) {
    logger.error() << "Subscription failed";
    QMetaObject::invokeMethod(m_handler, "stopSubscription", Qt::QueuedConnection);
    QMetaObject::invokeMethod(m_handler, "subscriptionCanceled", Qt::QueuedConnection);
  } else if (completedTransactionIds.isEmpty()) {
    Q_ASSERT(completedTransactions);
    logger.debug() << "Subscription completed - but all the transactions are known";
    QMetaObject::invokeMethod(m_handler, "stopSubscription", Qt::QueuedConnection);
    QMetaObject::invokeMethod(m_handler, "subscriptionCanceled", Qt::QueuedConnection);
  } else if (MozillaVPN::instance()->userAuthenticated()) {
    Q_ASSERT(completedTransactions);
    logger.debug() << "Subscription completed. Let's start the validation";
    QMetaObject::invokeMethod(m_handler, "processCompletedTransactions", Qt::QueuedConnection,
                              Q_ARG(QStringList, completedTransactionIds));
  } else {
    Q_ASSERT(completedTransactions);
    logger.debug() << "Subscription completed - but the user is not authenticated yet";
    QMetaObject::invokeMethod(m_handler, "stopSubscription", Qt::QueuedConnection);
    QMetaObject::invokeMethod(m_handler, "subscriptionCanceled", Qt::QueuedConnection);
  }

  for (SKPaymentTransaction* transaction in transactions) {
    switch (transaction.transactionState) {
      case SKPaymentTransactionStateFailed:
        [[fallthrough]];
      case SKPaymentTransactionStateRestored:
        [[fallthrough]];
      case SKPaymentTransactionStatePurchased:
        [queue finishTransaction:transaction];
        break;
      default:
        break;
    }
  }
}

- (void)requestDidFinish:(SKRequest*)request {
  logger.debug() << "Receipt refreshed correctly";
  QMetaObject::invokeMethod(m_handler, "stopSubscription", Qt::QueuedConnection);
  QMetaObject::invokeMethod(m_handler, "processCompletedTransactions", Qt::QueuedConnection,
                            Q_ARG(QStringList, QStringList()));
}

- (void)request:(SKRequest*)request didFailWithError:(NSError*)error {
  logger.error() << "Failed to refresh the receipt"
                 << QString::fromNSString(error.localizedDescription);
  QMetaObject::invokeMethod(m_handler, "stopSubscription", Qt::QueuedConnection);
  QMetaObject::invokeMethod(m_handler, "subscriptionFailed", Qt::QueuedConnection);
}

@end

// static
IAPHandler* IAPHandler::createInstance() {
  Q_ASSERT(!s_instance);
  new IAPHandler(qApp);
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

  m_delegate = [[IAPHandlerDelegate alloc] initWithObject:this];
  [[SKPaymentQueue defaultQueue]
      addTransactionObserver:static_cast<IAPHandlerDelegate*>(m_delegate)];
}

IAPHandler::~IAPHandler() {
  MVPN_COUNT_DTOR(IAPHandler);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;

  IAPHandlerDelegate* delegate = static_cast<IAPHandlerDelegate*>(m_delegate);
  [[SKPaymentQueue defaultQueue] removeTransactionObserver:delegate];

  [delegate dealloc];
  m_delegate = nullptr;
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

  for (const QJsonValue& value : products) {
    addProduct(value);
  }

  if (m_products.isEmpty()) {
    logger.error() << "No pending products (nothing has been registered). Unable to recover from "
                      "this scenario.";
    return;
  }

  NSSet<NSString*>* productIdentifiers = [NSSet<NSString*> set];
  for (const Product& product : m_products) {
    productIdentifiers = [productIdentifiers setByAddingObject:product.m_name.toNSString()];
  }

  logger.debug() << "We are about to register" << [productIdentifiers count] << "products";

  SKProductsRequest* productsRequest =
      [[SKProductsRequest alloc] initWithProductIdentifiers:productIdentifiers];

  IAPHandlerDelegate* delegate = static_cast<IAPHandlerDelegate*>(m_delegate);
  productsRequest.delegate = delegate;
  [productsRequest start];

  logger.debug() << "Waiting for the products registration";

  guard.dismiss();
}

void IAPHandler::addProduct(const QJsonValue& value) {
  if (!value.isObject()) {
    logger.debug() << "Object expected for the single product";
    return;
  }

  QJsonObject obj = value.toObject();

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

IAPHandler::Product* IAPHandler::findProduct(const QString& productIdentifier) {
  for (Product& p : m_products) {
    if (p.m_name == productIdentifier) {
      return &p;
    }
  }
  return nullptr;
}

void IAPHandler::startSubscription(const QString& productIdentifier) {
  Q_ASSERT(m_productsRegistrationState == eRegistered);

  Product* product = findProduct(productIdentifier);
  Q_ASSERT(product);
  Q_ASSERT(product->m_productNS);

  if (m_subscriptionState != eInactive) {
    logger.warning() << "No multiple IAP!";
    return;
  }

  m_subscriptionState = eActive;

  logger.debug() << "Starting the subscription";
  SKProduct* skProduct = static_cast<SKProduct*>(product->m_productNS);
  SKPayment* payment = [SKPayment paymentWithProduct:skProduct];
  [[SKPaymentQueue defaultQueue] addPayment:payment];
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

void IAPHandler::productRegistered(void* a_product) {
  SKProduct* product = static_cast<SKProduct*>(a_product);

  Q_ASSERT(m_productsRegistrationState == eRegistering);

  logger.debug() << "Product registered";

  NSString* nsProductIdentifier = [product productIdentifier];
  QString productIdentifier = QString::fromNSString(nsProductIdentifier);

  Product* productData = findProduct(productIdentifier);
  Q_ASSERT(productData);

  logger.debug() << "Id:" << productIdentifier;
  logger.debug() << "Title:" << QString::fromNSString([product localizedTitle]);
  logger.debug() << "Description:" << QString::fromNSString([product localizedDescription]);

  QString priceValue;
  {
    NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
    [numberFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
    [numberFormatter setNumberStyle:(NSNumberFormatterStyle)NSNumberFormatterCurrencyStyle];
    [numberFormatter setLocale:product.priceLocale];

    NSString* price = [numberFormatter stringFromNumber:product.price];
    priceValue = QString::fromNSString(price);
    [numberFormatter release];
  }

  logger.debug() << "Price:" << priceValue;

  QString monthlyPriceValue;
  NSDecimalNumber* monthlyPriceNS = nullptr;
  {
    NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
    [numberFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
    [numberFormatter setNumberStyle:(NSNumberFormatterStyle)NSNumberFormatterCurrencyStyle];
    [numberFormatter setLocale:product.priceLocale];

    int32_t mounthCount = productTypeToMonthCount(productData->m_type);
    Q_ASSERT(mounthCount >= 1);

    if (mounthCount == 1) {
      monthlyPriceNS = product.price;
    } else {
      NSDecimalNumber* divider = [[NSDecimalNumber alloc] initWithDouble:(double)mounthCount];
      monthlyPriceNS = [product.price decimalNumberByDividingBy:divider];
      [divider release];
    }

    NSString* price = [numberFormatter stringFromNumber:monthlyPriceNS];
    monthlyPriceValue = QString::fromNSString(price);

    [numberFormatter release];
  }

  logger.debug() << "Monthly Price:" << monthlyPriceValue;

  productData->m_price = priceValue;
  productData->m_monthlyPrice = monthlyPriceValue;
  productData->m_nonLocalizedMonthlyPrice = [monthlyPriceNS doubleValue];
  productData->m_productNS = product;
}

void IAPHandler::productsRegistrationCompleted() {
  logger.debug() << "All the products has been registered";

  beginResetModel();

  computeSavings();

  m_productsRegistrationState = eRegistered;

  endResetModel();

  emit productsRegistered();
}

void IAPHandler::processCompletedTransactions(const QStringList& ids) {
  logger.debug() << "process completed transactions";

  if (m_subscriptionState != eActive) {
    logger.warning() << "Random transaction to be completed. Let's ignore it";
    return;
  }

  QString receipt = IOSUtils::IAPReceipt();
  if (receipt.isEmpty()) {
    logger.warning() << "Empty receipt found";
    emit subscriptionFailed();
    return;
  }

  NetworkRequest* request = NetworkRequest::createForIOSPurchase(this, receipt);

  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Purchase request failed" << error;

            if (m_subscriptionState != eActive) {
              logger.warning() << "We have been canceled in the meantime";
              return;
            }

            stopSubscription();

            QJsonDocument json = QJsonDocument::fromJson(data);
            if (!json.isObject()) {
              MozillaVPN::instance()->errorHandle(ErrorHandler::toErrorType(error));
              emit subscriptionFailed();
              return;
            }

            QJsonObject obj = json.object();
            QJsonValue errorValue = obj.value("errno");
            if (!errorValue.isDouble()) {
              MozillaVPN::instance()->errorHandle(ErrorHandler::toErrorType(error));
              emit subscriptionFailed();
              return;
            }

            int errorNumber = errorValue.toInt();
            if (errorNumber != 145) {
              MozillaVPN::instance()->errorHandle(ErrorHandler::toErrorType(error));
              emit subscriptionFailed();
              return;
            }

            emit alreadySubscribed();
          });

  connect(request, &NetworkRequest::requestCompleted, [this, ids](const QByteArray&) {
    logger.debug() << "Purchase request completed";
    SettingsHolder::instance()->addSubscriptionTransactions(ids);

    if (m_subscriptionState != eActive) {
      logger.warning() << "We have been canceled in the meantime";
      return;
    }

    stopSubscription();
    emit subscriptionCompleted();
  });
}

void IAPHandler::subscribe(const QString& productIdentifier) {
  logger.debug() << "Subscription required";
  emit subscriptionStarted(productIdentifier);
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

    int savings = qRound(100.00 - ((product.m_nonLocalizedMonthlyPrice * 100.00) / monthlyPrice));
    if (savings < 0 || savings > 100) continue;

    product.m_savings = (int)savings;

    logger.debug() << "Saving" << product.m_savings << "for" << product.m_name;
  }
}

QHash<int, QByteArray> IAPHandler::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[ProductIdentifierRole] = "productIdentifier";
  roles[ProductPriceRole] = "productPrice";
  roles[ProductMonthlyPriceRole] = "productMonthlyPrice";
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

    default:
      return QVariant();
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
