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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

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
    m_handler->unknownProductRegistered(QString::fromNSString(identifier));
  } else {
    SKProduct* product = [[products firstObject] retain];
    m_handler->productRegistered(product);
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
        logger.log() << "transaction restored or purchased - identifier: " << identifier
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

  m_handler->stopSubscription();

  if (canceledTransactions) {
    logger.log() << "Subscription canceled";
    emit m_handler->subscriptionCanceled();
  } else if (failedTransactions) {
    logger.log() << "Subscription failed";
    emit m_handler->subscriptionFailed();
  } else if (completedTransactionIds.isEmpty()) {
    Q_ASSERT(completedTransactions);
    logger.log() << "Subscription completed - but all the transactions are known";
    emit m_handler->subscriptionFailed();
  } else if (MozillaVPN::instance()->userAuthenticated()) {
    Q_ASSERT(completedTransactions);
    logger.log() << "Subscription completed. Let's start the validation";
    m_handler->processCompletedTransactions(completedTransactionIds);
  } else {
    Q_ASSERT(completedTransactions);
    logger.log() << "Subscription completed - but the user is not authenticated yet";
    emit m_handler->subscriptionFailed();
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
  logger.log() << "Recept refreshed correctly";
  m_handler->stopSubscription();
  m_handler->processCompletedTransactions(QStringList());
}

- (void)request:(SKRequest*)request didFailWithError:(NSError*)error {
  logger.log() << "Failed to refresh the receipt"
               << QString::fromNSString(error.localizedDescription);
  m_handler->stopSubscription();
  emit m_handler->subscriptionFailed();
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

IAPHandler::IAPHandler(QObject* parent)
    : QObject(parent), m_priceValue(Constants::SUBSCRIPTION_CURRENCY_VALUE_USD) {
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

void IAPHandler::registerProducts(const QStringList& products) {
  logger.log() << "Maybe register products";

  Q_ASSERT(!products.isEmpty());
  Q_ASSERT(products.length() == 1);
  Q_ASSERT(m_productsRegistrationState == eRegistered ||
           m_productsRegistrationState == eNotRegistered);

  if (m_productsRegistrationState == eRegistered) {
    emit productsRegistered();
    return;
  }

  m_productsRegistrationState = eRegistering;

  m_productName = products.at(0);
  logger.log() << "Registration product:" << m_productName;

  IAPHandlerDelegate* delegate = static_cast<IAPHandlerDelegate*>(m_delegate);

  NSSet<NSString*>* productId = [NSSet<NSString*> setWithObject:m_productName.toNSString()];
  SKProductsRequest* productsRequest =
      [[SKProductsRequest alloc] initWithProductIdentifiers:productId];
  productsRequest.delegate = delegate;
  [productsRequest start];

  logger.log() << "Waiting for the products registration";
}

void IAPHandler::startSubscription(bool restore) {
  Q_ASSERT(m_productsRegistrationState == eRegistered);
  Q_ASSERT(!m_productName.isEmpty());

  if (m_subscriptionState != eInactive) {
    logger.log() << "No multiple IAP!";
    return;
  }

  if (!m_product) {
    logger.log() << "No product registered";

    // The product registration failed, for unknown reasons.
    emit subscriptionFailed();
    return;
  }

  m_subscriptionState = eActive;

  if (restore) {
    logger.log() << "Restore the subscription";
    SKReceiptRefreshRequest* refresh =
        [[SKReceiptRefreshRequest alloc] initWithReceiptProperties:nil];
    refresh.delegate = static_cast<IAPHandlerDelegate*>(m_delegate);
    [refresh start];
    return;
  }

  logger.log() << "Starting the subscription";
  SKProduct* product = static_cast<SKProduct*>(m_product);
  SKPayment* payment = [SKPayment paymentWithProduct:product];
  [[SKPaymentQueue defaultQueue] addPayment:payment];
}

void IAPHandler::stopSubscription() { m_subscriptionState = eInactive; }

void IAPHandler::unknownProductRegistered(const QString& identifier) {
  Q_ASSERT(m_productsRegistrationState == eRegistering);
  m_productsRegistrationState = eRegistered;

  logger.log() << "Product registration failed:" << identifier;

  emit productsRegistered();
}

void IAPHandler::productRegistered(void* a_product) {
  SKProduct* product = static_cast<SKProduct*>(a_product);

  Q_ASSERT(m_productsRegistrationState == eRegistering);
  m_productsRegistrationState = eRegistered;

  Q_ASSERT(!m_product);
  m_product = product;

  logger.log() << "Product registered";
  logger.log() << "Title:" << QString::fromNSString([product localizedTitle]);
  logger.log() << "Description:" << QString::fromNSString([product localizedDescription]);

  NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
  [numberFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
  [numberFormatter setNumberStyle:(NSNumberFormatterStyle)NSNumberFormatterCurrencyStyle];
  [numberFormatter setLocale:product.priceLocale];

  NSString* price = [numberFormatter stringFromNumber:product.price];
  m_priceValue = QString::fromNSString(price);
  logger.log() << "Price:" << m_priceValue;
  [numberFormatter release];

  emit priceValueChanged();
  emit productsRegistered();
}

void IAPHandler::processCompletedTransactions(const QStringList& ids) {
  logger.log() << "process completed transactions";

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

            QJsonDocument json = QJsonDocument::fromJson(data);
            if (!json.isObject()) {
              MozillaVPN::instance()->errorHandle(ErrorHandler::toErrorType(error));
              emit subscriptionFailed();
              return;
            }

            QJsonObject obj = json.object();
            QJsonValue errorValue = obj.take("errno");
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
    emit subscriptionCompleted();
  });
}

void IAPHandler::subscribe() {
  logger.log() << "Subscription required";
  emit subscriptionStarted(false /* restore */);
}

void IAPHandler::restoreSubscription() {
  logger.log() << "Restore subscription";
  emit subscriptionStarted(true /* restore */);
}
