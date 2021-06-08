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
  NSArray<SKProduct*>* products = response.products;
  logger.log() << "Products registered";

  if ([products count] != 1) {
    NSString* identifier = [response.invalidProductIdentifiers firstObject];
    QMetaObject::invokeMethod(m_handler, "unknownProductRegistered", Qt::QueuedConnection,
                              Q_ARG(QString, QString::fromNSString(identifier)));
  } else {
    SKProduct* product = [[products firstObject] retain];
    QMetaObject::invokeMethod(m_handler, "productRegistered", Qt::QueuedConnection,
                              Q_ARG(void*, product));
  }

  [request release];
}

- (void)paymentQueue:(nonnull SKPaymentQueue*)queue
    updatedTransactions:(nonnull NSArray<SKPaymentTransaction*>*)transactions {
  logger.log() << "payment queue:" << [transactions count];

  QStringList completedTransactionIds;
  bool failedTransactions = false;
  bool canceledTransactions = false;
  bool completedTransactions = false;

  for (SKPaymentTransaction* transaction in transactions) {
    switch (transaction.transactionState) {
      case SKPaymentTransactionStateFailed:
        logger.log() << "transaction failed";

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
        logger.log() << "transaction purchased - identifier: " << identifier
                     << "- date:" << date.toString();

        if (transaction.transactionState == SKPaymentTransactionStateRestored) {
          SKPaymentTransaction* originalTransaction = transaction.originalTransaction;
          if (originalTransaction) {
            QString originalIdentifier =
                QString::fromNSString(originalTransaction.transactionIdentifier);
            QDateTime originalDate = QDateTime::fromNSDate(originalTransaction.transactionDate);
            logger.log() << "original transaction identifier: " << originalIdentifier
                         << "- date:" << originalDate.toString();
          }
        }

        completedTransactions = true;

        SettingsHolder* settingsHolder = SettingsHolder::instance();
        if (settingsHolder->hasSubscriptionTransaction(identifier)) {
          logger.log() << "This transaction has already been processed. Let's ignore it.";
        } else {
          completedTransactionIds.append(identifier);
        }

        break;
      }
      case SKPaymentTransactionStatePurchasing:
        logger.log() << "transaction purchasing";
        break;
      case SKPaymentTransactionStateDeferred:
        logger.log() << "transaction deferred";
        break;
      default:
        logger.log() << "transaction unknwon state";
        break;
    }
  }

  if (!completedTransactions && !canceledTransactions && !failedTransactions) {
    // Nothing completed, nothing restored, nothing failed. Just purchasing transactions.
    return;
  }

  if (canceledTransactions) {
    logger.log() << "Subscription canceled";
    QMetaObject::invokeMethod(m_handler, "stopSubscription", Qt::QueuedConnection);
    QMetaObject::invokeMethod(m_handler, "subscriptionCanceled", Qt::QueuedConnection);
  } else if (failedTransactions) {
    logger.log() << "Subscription failed";
    QMetaObject::invokeMethod(m_handler, "stopSubscription", Qt::QueuedConnection);
    QMetaObject::invokeMethod(m_handler, "subscriptionCanceled", Qt::QueuedConnection);
  } else if (completedTransactionIds.isEmpty()) {
    Q_ASSERT(completedTransactions);
    logger.log() << "Subscription completed - but all the transactions are known";
    QMetaObject::invokeMethod(m_handler, "stopSubscription", Qt::QueuedConnection);
    QMetaObject::invokeMethod(m_handler, "subscriptionCanceled", Qt::QueuedConnection);
  } else if (MozillaVPN::instance()->userAuthenticated()) {
    Q_ASSERT(completedTransactions);
    logger.log() << "Subscription completed. Let's start the validation";
    QMetaObject::invokeMethod(m_handler, "processCompletedTransactions", Qt::QueuedConnection,
                              Q_ARG(QStringList, completedTransactionIds));
  } else {
    Q_ASSERT(completedTransactions);
    logger.log() << "Subscription completed - but the user is not authenticated yet";
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
  logger.log() << "Receipt refreshed correctly";
  QMetaObject::invokeMethod(m_handler, "stopSubscription", Qt::QueuedConnection);
  QMetaObject::invokeMethod(m_handler, "processCompletedTransactions", Qt::QueuedConnection,
                            Q_ARG(QStringList, QStringList()));
}

- (void)request:(SKRequest*)request didFailWithError:(NSError*)error {
  logger.log() << "Failed to refresh the receipt"
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
  logger.log() << "Maybe register products";

  Q_ASSERT(m_productsRegistrationState == eRegistered ||
           m_productsRegistrationState == eNotRegistered);

  auto guard = qScopeGuard([&] { emit productsRegistered(); });

  if (m_productsRegistrationState == eRegistered) {
    return;
  }

  Q_ASSERT(m_registeredProducts.isEmpty());

  QJsonDocument json = QJsonDocument::fromJson(data);
  if (!json.isObject()) {
    logger.log() << "Object expected";
    return;
  }

  QJsonObject obj = json.object();
  if (!obj.contains("products")) {
    logger.log() << "products entry expected";
    return;
  }

  QJsonArray products = obj["products"].toArray();
  if (products.isEmpty()) {
    logger.log() << "No products found";
    return;
  }

  m_productsRegistrationState = eRegistering;

  for (const QJsonValue& value : products) {
    registerProduct(value);
  }

  if (m_pendingProducts.isEmpty()) {
    logger.log() << "No pending products";
    return;
  }

  logger.log() << "Waiting for the products registration";

  guard.dismiss();
}

void IAPHandler::registerProduct(const QJsonValue& value) {
  if (!value.isObject()) {
    logger.log() << "Object expected for the single product";
    return;
  }

  QJsonObject obj = value.toObject();

  Product product;
  product.m_name = obj["id"].toString();
  product.m_price = Constants::SUBSCRIPTION_CURRENCY_VALUE_USD;
  product.m_type = productTypeToEnum(obj["type"].toString());
  if (product.m_type == ProductUnknown) {
    logger.log() << "Unknown product type:" << obj["type"].toString();
    return;
  }

  m_pendingProducts.insert(product.m_name, product);

  logger.log() << "Registration product:" << product.m_name;

  IAPHandlerDelegate* delegate = static_cast<IAPHandlerDelegate*>(m_delegate);

  NSSet<NSString*>* productIdentifier =
      [NSSet<NSString*> setWithObject:product.m_name.toNSString()];
  SKProductsRequest* productsRequest =
      [[SKProductsRequest alloc] initWithProductIdentifiers:productIdentifier];
  productsRequest.delegate = delegate;
  [productsRequest start];
}

void IAPHandler::startSubscription(const QString& productIdentifier) {
  Q_ASSERT(m_productsRegistrationState == eRegistered);

  Product* product = nullptr;
  for (Product& p : m_registeredProducts) {
    if (p.m_name == productIdentifier) {
      product = &p;
      break;
    }
  }

  Q_ASSERT(product);
  Q_ASSERT(product->m_product);

  if (m_subscriptionState != eInactive) {
    logger.log() << "No multiple IAP!";
    return;
  }

  m_subscriptionState = eActive;

  logger.log() << "Starting the subscription";
  SKProduct* skProduct = static_cast<SKProduct*>(product->m_product);
  SKPayment* payment = [SKPayment paymentWithProduct:skProduct];
  [[SKPaymentQueue defaultQueue] addPayment:payment];
}

void IAPHandler::stopSubscription() {
  logger.log() << "Stop subscription";
  m_subscriptionState = eInactive;
}

void IAPHandler::unknownProductRegistered(const QString& identifier) {
  Q_ASSERT(m_productsRegistrationState == eRegistering);
  Q_ASSERT(!m_pendingProducts.isEmpty());

  logger.log() << "Product registration failed:" << identifier;

  Q_ASSERT(m_pendingProducts.contains(identifier));
  m_pendingProducts.remove(identifier);

  if (m_pendingProducts.isEmpty()) {
    logger.log() << "All the products has been registed (with failures)";
    m_productsRegistrationState = eRegistered;
    sortProductsAndCompleteRegistration();
  }
}

void IAPHandler::productRegistered(void* a_product) {
  SKProduct* product = static_cast<SKProduct*>(a_product);

  Q_ASSERT(m_productsRegistrationState == eRegistering);
  Q_ASSERT(!m_pendingProducts.isEmpty());

  logger.log() << "Product registered";

  NSString* nsProductIdentifier = [product productIdentifier];
  QString productIdentifier = QString::fromNSString(nsProductIdentifier);

  logger.log() << "Id:" << productIdentifier;
  logger.log() << "Title:" << QString::fromNSString([product localizedTitle]);
  logger.log() << "Description:" << QString::fromNSString([product localizedDescription]);

  NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
  [numberFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
  [numberFormatter setNumberStyle:(NSNumberFormatterStyle)NSNumberFormatterCurrencyStyle];
  [numberFormatter setLocale:product.priceLocale];

  NSString* price = [numberFormatter stringFromNumber:product.price];
  QString priceValue = QString::fromNSString(price);

  logger.log() << "Price:" << priceValue;
  [numberFormatter release];

  Q_ASSERT(m_pendingProducts.contains(productIdentifier));
  Product& productData = m_pendingProducts[productIdentifier];
  productData.m_price = priceValue;
  productData.m_product = product;

  m_registeredProducts.append(productData);
  m_pendingProducts.remove(productIdentifier);

  if (m_pendingProducts.isEmpty()) {
    logger.log() << "All the products has been registered";
    m_productsRegistrationState = eRegistered;
    sortProductsAndCompleteRegistration();
  }
}

void IAPHandler::processCompletedTransactions(const QStringList& ids) {
  logger.log() << "process completed transactions";

  if (m_subscriptionState != eActive) {
    logger.log() << "Random transaction to be completed. Let's ignore it";
    return;
  }

  QString receipt = IOSUtils::IAPReceipt();
  if (receipt.isEmpty()) {
    logger.log() << "Empty receipt found";
    emit subscriptionFailed();
    return;
  }

  NetworkRequest* request = NetworkRequest::createForIOSPurchase(this, receipt);

  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.log() << "Purchase request failed" << error;

            if (m_subscriptionState != eActive) {
              logger.log() << "We have been canceled in the meantime";
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
    logger.log() << "Purchase request completed";
    SettingsHolder::instance()->addSubscriptionTransactions(ids);

    if (m_subscriptionState != eActive) {
      logger.log() << "We have been canceled in the meantime";
      return;
    }

    stopSubscription();
    emit subscriptionCompleted();
  });
}

void IAPHandler::subscribe(const QString& productIdentifier) {
  logger.log() << "Subscription required";
  emit subscriptionStarted(productIdentifier);
}

QHash<int, QByteArray> IAPHandler::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[ProductIdentifierRole] = "productIdentifier";
  roles[ProductPriceRole] = "productPrice";
  roles[ProductTypeRole] = "productType";
  return roles;
}

int IAPHandler::rowCount(const QModelIndex&) const { return m_registeredProducts.count(); }

QVariant IAPHandler::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  switch (role) {
    case ProductIdentifierRole:
      return QVariant(m_registeredProducts.at(index.row()).m_name);

    case ProductPriceRole:
      return QVariant(m_registeredProducts.at(index.row()).m_price);

    case ProductTypeRole:
      return QVariant(m_registeredProducts.at(index.row()).m_type);

    default:
      return QVariant();
  }
}

// static
bool IAPHandler::sortProductsCallback(const IAPHandler::Product& a, const IAPHandler::Product& b) {
  return a.m_type >= b.m_type;
}

void IAPHandler::sortProductsAndCompleteRegistration() {
  std::sort(m_registeredProducts.begin(), m_registeredProducts.end(), sortProductsCallback);
  emit productsRegistered();
}

// static
IAPHandler::ProductType IAPHandler::productTypeToEnum(const QString& type) {
  if (type == "yearly") return ProductYearly;
  if (type == "monthly") return ProductMonthly;
  return ProductUnknown;
}
