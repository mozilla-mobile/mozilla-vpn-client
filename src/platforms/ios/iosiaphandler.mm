/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/ios/iosiaphandler.h"
#include "Mozilla-Swift.h"

#include "app.h"
#include "errorhandler.h"
#include "iosutils.h"
#include "leakdetector.h"
#include "logger.h"
#include "settingsholder.h"
#include "tasks/purchase/taskpurchase.h"
#include "taskscheduler.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>

constexpr const uint32_t GUARDIAN_ERROR_RECEIPT_NOT_VALID = 142;
constexpr const uint32_t GUARDIAN_ERROR_RECEIPT_IN_USE = 145;

namespace {
Logger logger("IOSIAPHandler");
bool s_transactionsProcessed = false;
}  // namespace

@interface IOSIAPHandlerDelegate
    : NSObject <SKRequestDelegate, SKProductsRequestDelegate, SKPaymentTransactionObserver> {
  IOSIAPHandler* m_handler;
}
@end

@implementation IOSIAPHandlerDelegate

- (id)initWithObject:(IOSIAPHandler*)handler {
  self = [super init];
  if (self) {
    m_handler = handler;
  }
  return self;
}

- (void)productsRequest:(nonnull SKProductsRequest*)request
     didReceiveResponse:(nonnull SKProductsResponse*)response {
  logger.debug() << "Registration completed";

  ProductsHandler* productsHandler = ProductsHandler::instance();
  PurchaseIAPHandler* purchaseHandler = IOSIAPHandler::instance();

  if (response.invalidProductIdentifiers) {
    NSArray<NSString*>* products = response.invalidProductIdentifiers;
    logger.error() << "Registration failure" << [products count];

    for (unsigned long i = 0, count = [products count]; i < count; ++i) {
      NSString* identifier = [products objectAtIndex:i];
      QMetaObject::invokeMethod(productsHandler, "unknownProductRegistered", Qt::QueuedConnection,
                                Q_ARG(QString, QString::fromNSString(identifier)));
    }
  }

  NSArray<SKProduct*>* products = response.products;
  if (products) {
    logger.debug() << "Products registered" << [products count];

    for (unsigned long i = 0, count = [products count]; i < count; ++i) {
      SKProduct* product = [[products objectAtIndex:i] retain];
      QMetaObject::invokeMethod(purchaseHandler, "productRegistered", Qt::QueuedConnection,
                                Q_ARG(void*, product));
    }
  }

  QMetaObject::invokeMethod(productsHandler, "productsRegistrationCompleted", Qt::QueuedConnection);

  [request release];
}

- (void)paymentQueue:(nonnull SKPaymentQueue*)queue
    updatedTransactions:(nonnull NSArray<SKPaymentTransaction*>*)transactions {
  logger.debug() << "payment queue:" << [transactions count];

  s_transactionsProcessed = true;

  QStringList completedTransactionIds;
  bool failedTransactions = false;
  bool canceledTransactions = false;
  bool completedTransactions = false;

  for (SKPaymentTransaction* transaction in transactions) {
    switch (transaction.transactionState) {
      case SKPaymentTransactionStateFailed:
        logger.error() << "transaction failed: " << transaction.error.code;

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

        if (SettingsHolder::instance()->subscriptionTransactions().contains(identifier)) {
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
        logger.warning() << "transaction unknown state";
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
  } else if (App::instance()->userAuthenticated()) {
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

- (void)paymentQueueRestoreCompletedTransactionsFinished:(nonnull SKPaymentQueue*)queue {
  if (!s_transactionsProcessed) {
    logger.error() << "No transaction to restore";
    QMetaObject::invokeMethod(m_handler, "noSubscriptionFoundError", Qt::QueuedConnection);
  }
  s_transactionsProcessed = false;
  logger.debug() << "restore request completed";
}

@end

IOSIAPHandler::IOSIAPHandler(QObject* parent) : PurchaseIAPHandler(parent) {
  MZ_COUNT_CTOR(IOSIAPHandler);

  if (@available(iOS 15, *)) {
    swiftIAPHandler = [[InAppPurchaseHandler alloc]
        initWithErrorCallback:^(void) {
          logger.debug() << "Subscription error with StoreKit2.";
          QMetaObject::invokeMethod(this, "stopSubscription", Qt::QueuedConnection);
          QMetaObject::invokeMethod(this, "subscriptionCanceled", Qt::QueuedConnection);
        }
        successCallback:^(NSString* productIdentifier, NSString* transactionIdentifier) {
          if (App::instance()->userAuthenticated()) {
            logger.debug() << "Subscription completed with StoreKit2. Starting validation.";
            QMetaObject::invokeMethod(
                this, "processCompletedTransactions", Qt::QueuedConnection,
                Q_ARG(QStringList, {QString::fromNSString(productIdentifier)}),
                Q_ARG(QString, QString::fromNSString(transactionIdentifier)));
          } else {
            logger.debug() << "Subscription completed with StoreKit2. User not signed in.";
            QMetaObject::invokeMethod(this, "stopSubscription", Qt::QueuedConnection);
            QMetaObject::invokeMethod(this, "subscriptionCanceled", Qt::QueuedConnection);
          }
        }];
  } else {
    m_delegate = [[IOSIAPHandlerDelegate alloc] initWithObject:this];
    [[SKPaymentQueue defaultQueue]
        addTransactionObserver:static_cast<IOSIAPHandlerDelegate*>(m_delegate)];
  }
}

IOSIAPHandler::~IOSIAPHandler() {
  MZ_COUNT_DTOR(IOSIAPHandler);

  IOSIAPHandlerDelegate* delegate = static_cast<IOSIAPHandlerDelegate*>(m_delegate);
  [[SKPaymentQueue defaultQueue] removeTransactionObserver:delegate];

  [delegate dealloc];
  m_delegate = nullptr;
  if (@available(iOS 15, *)) {
    swiftIAPHandler = nullptr;
  }
}

void IOSIAPHandler::nativeRegisterProducts() {
  NSSet<NSString*>* productIdentifiers = [NSSet<NSString*> set];
  for (const ProductsHandler::Product& product : ProductsHandler::instance()->products()) {
    productIdentifiers = [productIdentifiers setByAddingObject:product.m_name.toNSString()];
  }

  logger.debug() << "We are about to register" << [productIdentifiers count] << "products...";

  if (@available(iOS 15, *)) {
    logger.debug() << "...using StoreKit2 API.";
    [(InAppPurchaseHandler*)swiftIAPHandler getProductsWith:productIdentifiers
        productRegistrationCallback:^(NSString* productIdentifier, NSString* currencyCode,
                                      NSString* totalPrice, NSString* monthlyPrice,
                                      double monthlyPriceNumber, NSInteger freeTrialDays) {
          ProductsHandler* productsHandler = ProductsHandler::instance();
          Q_ASSERT(productsHandler->isRegistering());
          logger.debug() << "Product registered";
          ProductsHandler::Product* productData =
              productsHandler->findProduct(QString::fromNSString(productIdentifier));
          Q_ASSERT(productData);
          productData->m_price = QString::fromNSString(totalPrice);
          productData->m_trialDays = (int)freeTrialDays;
          productData->m_monthlyPrice = QString::fromNSString(monthlyPrice);
          productData->m_nonLocalizedMonthlyPrice = monthlyPriceNumber;
          productData->m_currencyCode = QString::fromNSString(currencyCode);

          logger.debug() << "Id:" << QString::fromNSString(productIdentifier);
          logger.debug() << "Price:" << productData->m_price;
          logger.debug() << "Monthly price:" << productData->m_monthlyPrice;
        }
        registrationCompleteCallback:^(void) {
          ProductsHandler* productsHandler = ProductsHandler::instance();
          QMetaObject::invokeMethod(productsHandler, "productsRegistrationCompleted",
                                    Qt::QueuedConnection);
        }
        registrationFailureCallback:^{
          logger.error() << "Registration failure";
          ProductsHandler* productsHandler = ProductsHandler::instance();
          NSArray* productIdentifiersArray = [productIdentifiers allObjects];
          for (unsigned long i = 0, count = [productIdentifiersArray count]; i < count; ++i) {
            NSString* identifier = [productIdentifiersArray objectAtIndex:i];
            productsHandler->unknownProductRegistered(QString::fromNSString(identifier));
          }
        }
        completionHandler:^{
        }];
  } else {
    logger.debug() << "...using legacy StoreKit API.";
    SKProductsRequest* productsRequest =
        [[SKProductsRequest alloc] initWithProductIdentifiers:productIdentifiers];

    IOSIAPHandlerDelegate* delegate = static_cast<IOSIAPHandlerDelegate*>(m_delegate);
    productsRequest.delegate = delegate;
    [productsRequest start];
  }
}

void IOSIAPHandler::nativeStartSubscription(ProductsHandler::Product* product) {
  if (@available(iOS 15, *)) {
    logger.debug() << "Using StoreKit2 APIs";
    NSString* productId = product->m_name.toNSString();
    [(InAppPurchaseHandler*)swiftIAPHandler startSubscriptionFor:productId
                                               completionHandler:^{
                                               }];
  } else {
    logger.debug() << "Using legacy StoreKit API.";
    Q_ASSERT(product->m_extra);
    SKProduct* skProduct = static_cast<SKProduct*>(product->m_extra);
    SKPayment* payment = [SKPayment paymentWithProduct:skProduct];
    [[SKPaymentQueue defaultQueue] addPayment:payment];
  }
}

void IOSIAPHandler::nativeRestoreSubscription() {
  if (@available(iOS 15, *)) {
    [(InAppPurchaseHandler*)swiftIAPHandler restoreSubscriptionsWithCompletionHandler:^{
    }];
  } else {
    s_transactionsProcessed = false;
    [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
  }
}

void IOSIAPHandler::productRegistered(void* a_product) {
  SKProduct* product = static_cast<SKProduct*>(a_product);

  ProductsHandler* productsHandler = ProductsHandler::instance();

  Q_ASSERT(productsHandler->isRegistering());

  logger.debug() << "Product registered";

  NSString* nsProductIdentifier = [product productIdentifier];
  QString productIdentifier = QString::fromNSString(nsProductIdentifier);

  ProductsHandler::Product* productData = productsHandler->findProduct(productIdentifier);
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

    int32_t mounthCount = productsHandler->productTypeToMonthCount(productData->m_type);
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
  int discountDays = 0;
  if (@available(iOS 12.2, *)) {
    auto discount = product.introductoryPrice;
    discountDays = discountToDays(discount);
  }

  logger.debug() << "Monthly Price:" << monthlyPriceValue;

  productData->m_price = priceValue;
  productData->m_trialDays = discountDays;
  productData->m_monthlyPrice = monthlyPriceValue;
  productData->m_nonLocalizedMonthlyPrice = [monthlyPriceNS doubleValue];
  productData->m_currencyCode = QString::fromNSString(product.priceLocale.currencyCode);
  productData->m_extra = product;
}

// Called directly when using StoreKit1
void IOSIAPHandler::processCompletedTransactions(const QStringList& ids) {
  if (@available(iOS 15, *)) {
    logger.error() << "This block should never be hit.";
    assert(NO);
  } else {
    // For StoreKit1, we don't need a transaction identifier, so giving it some garbage.
    processCompletedTransactions(ids, "this should never be seen");
  }
}

// Called directly when using StoreKit2
void IOSIAPHandler::processCompletedTransactions(const QStringList& ids, const QString transactionIdentifier) {
  logger.debug() << "process completed transactions";

  if (m_subscriptionState != eActive) {
    logger.warning() << "Completing transaction out of subscription process!";
  }

  TaskPurchase* purchase;
  if (@available(iOS 15, *)) {
    purchase = TaskPurchase::createForIOS(transactionIdentifier, false);
  } else {
    QString receipt = IOSUtils::IAPReceipt();
    if (receipt.isEmpty()) {
      logger.warning() << "Empty receipt found";
      emit subscriptionFailed();
      return;
    }
    purchase = TaskPurchase::createForIOS(receipt, true);
  }

  Q_ASSERT(purchase);

  connect(
      purchase, &TaskPurchase::failed, this,
      [purchase, this](QNetworkReply::NetworkError error, const QByteArray& data) {
        logger.error() << "Purchase request failed" << error;

        stopSubscription();

        QJsonDocument json = QJsonDocument::fromJson(data);
        if (!json.isObject()) {
          REPORTNETWORKERROR(error, ErrorHandler::PropagateError, purchase->name());
          emit subscriptionFailed();
          emit ErrorHandler::instance()->subscriptionGeneric();
          return;
        }

        QJsonObject obj = json.object();
        QJsonValue errorValue = obj.value("errno");
        if (!errorValue.isDouble()) {
          REPORTNETWORKERROR(error, ErrorHandler::PropagateError, purchase->name());
          emit subscriptionFailed();
          emit ErrorHandler::instance()->subscriptionGeneric();
          return;
        }

        int errorNumber = errorValue.toInt();
        if (errorNumber == GUARDIAN_ERROR_RECEIPT_NOT_VALID) {
          REPORTNETWORKERROR(error, ErrorHandler::PropagateError, purchase->name());
          emit subscriptionFailed();
          emit ErrorHandler::instance()->subscriptionExpired();
          return;
        }

        if (errorNumber == GUARDIAN_ERROR_RECEIPT_IN_USE) {
          REPORTNETWORKERROR(error, ErrorHandler::PropagateError, purchase->name());
          emit subscriptionFailed();
          emit ErrorHandler::instance()->subscriptionInUse();
          return;
        }

        emit alreadySubscribed();
      });

  connect(purchase, &TaskPurchase::succeeded, this, [this, ids](const QByteArray&) {
    logger.debug() << "Purchase request completed";
    SettingsHolder* settingsHolder = SettingsHolder::instance();
    Q_ASSERT(settingsHolder);

    QStringList transactions = settingsHolder->subscriptionTransactions();
    transactions.append(ids);
    settingsHolder->setSubscriptionTransactions(transactions);

    stopSubscription();
    emit subscriptionCompleted();
  });

  TaskScheduler::scheduleTask(purchase);
}

void IOSIAPHandler::noSubscriptionFoundError() {
  emit subscriptionCanceled();
  emit ErrorHandler::instance()->noSubscriptionFound();
}

int IOSIAPHandler::discountToDays(void* aDiscount) {
  SKProductDiscount* discount = static_cast<SKProductDiscount*>(aDiscount);
  if (discount == nullptr) {
    return 0;
  }
  if (discount.paymentMode != SKProductDiscountPaymentMode::SKProductDiscountPaymentModeFreeTrial) {
    return 0;
  }
  // Is it a week / day / month
  auto discountPeriodUnit = discount.subscriptionPeriod.unit;
  // How many units (i.e 3 days) per period
  auto periodUnits = (int)discount.subscriptionPeriod.numberOfUnits;
  // How many period's are we getting
  auto discountAmount = (int)discount.numberOfPeriods;
  switch (discountPeriodUnit) {
    case SKProductPeriodUnitDay:
      return discountAmount * periodUnits;
    case SKProductPeriodUnitWeek:
      return 7 * discountAmount * periodUnits;
    case SKProductPeriodUnitMonth:
      return 30 * discountAmount * periodUnits;
    default:
      Q_UNREACHABLE();
      return 0;
  }
  Q_UNREACHABLE();
  return 0;
}
