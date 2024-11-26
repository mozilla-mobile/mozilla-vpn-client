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

constexpr const uint32_t GUARDIAN_ERROR_RECEIPT_NOT_VALID = 142;
constexpr const uint32_t GUARDIAN_ERROR_RECEIPT_IN_USE = 145;

namespace {
Logger logger("IOSIAPHandler");
}  // namespace

IOSIAPHandler::IOSIAPHandler(QObject* parent) : PurchaseIAPHandler(parent) {
  MZ_COUNT_CTOR(IOSIAPHandler);

  swiftIAPHandler = [[InAppPurchaseHandler alloc]
      initWithErrorCallback:^(bool showNoSubscriptionError) {
        logger.debug() << "Subscription error with StoreKit2.";
        QMetaObject::invokeMethod(this, "stopSubscription", Qt::QueuedConnection);
        QMetaObject::invokeMethod(this, "subscriptionCanceled", Qt::QueuedConnection);

        if (showNoSubscriptionError) {
          emit ErrorHandler::instance()->noSubscriptionFound();
        }
      }
      successCallback:^(NSString* productIdentifier, NSString* transactionIdentifier) {
        if (App::instance()->userAuthenticated()) {
          logger.debug() << "Subscription completed with StoreKit2. Starting validation.";
          QMetaObject::invokeMethod(this, "processCompletedTransactions", Qt::QueuedConnection,
                                    Q_ARG(QStringList, {QString::fromNSString(productIdentifier)}),
                                    Q_ARG(QString, QString::fromNSString(transactionIdentifier)));
        } else {
          logger.debug() << "Subscription completed with StoreKit2. User not signed in.";
          QMetaObject::invokeMethod(this, "stopSubscription", Qt::QueuedConnection);
          QMetaObject::invokeMethod(this, "subscriptionCanceled", Qt::QueuedConnection);
        }
      }];
}

IOSIAPHandler::~IOSIAPHandler() {
  MZ_COUNT_DTOR(IOSIAPHandler);
  swiftIAPHandler = nullptr;
}

void IOSIAPHandler::nativeRegisterProducts() {
  NSSet<NSString*>* productIdentifiers = [NSSet<NSString*> set];
  for (const ProductsHandler::Product& product : ProductsHandler::instance()->products()) {
    productIdentifiers = [productIdentifiers setByAddingObject:product.m_name.toNSString()];
  }

  logger.debug() << "Registering" << [productIdentifiers count] << "products using StoreKit2 API.";
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
}

void IOSIAPHandler::nativeStartSubscription(ProductsHandler::Product* product) {
  logger.debug() << "Using StoreKit2 APIs";
  NSString* productId = product->m_name.toNSString();
  [(InAppPurchaseHandler*)swiftIAPHandler startSubscriptionFor:productId
                                             completionHandler:^{
                                             }];
}

void IOSIAPHandler::nativeRestoreSubscription() {
  [(InAppPurchaseHandler*)swiftIAPHandler restoreSubscriptionsWithCompletionHandler:^{
  }];
}

void IOSIAPHandler::processCompletedTransactions(const QStringList& ids,
                                                 const QString transactionIdentifier) {
  logger.debug() << "process completed transactions";

  if (m_subscriptionState != eActive) {
    logger.warning() << "Completing transaction out of subscription process!";
  }

  TaskPurchase* purchase = TaskPurchase::createForIOS(transactionIdentifier, false);

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
