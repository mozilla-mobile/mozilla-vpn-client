/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/ios/iaphandler.h"
#include "iosutils.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QCoreApplication>
#include <QInAppStore>
#include <QtPurchasing>

namespace {
Logger logger(LOG_IAP, "IAPHandler");

IAPHandler* s_instance = nullptr;
}  // namespace

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

IAPHandler::IAPHandler(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(IAPHandler);

  Q_ASSERT(!s_instance);
  s_instance = this;

  connect(&m_appStore, &QInAppStore::productRegistered,
          [this](QInAppProduct* product) {
            Q_ASSERT(m_productsRegistrationState == eRegistering);
            m_productsRegistrationState = eRegistered;

            logger.log() << "Product registered";
            logger.log() << "Title:" << product->title();
            logger.log() << "Description:" << product->description();
            logger.log() << "Price:" << product->price();

            emit productsRegistered();
          });

    connect(&m_appStore,
            &QInAppStore::productUnknown,
            [this](QInAppProduct::ProductType productType, const QString &identifier) {
    Q_ASSERT(m_productsRegistrationState == eRegistering);
    m_productsRegistrationState = eRegistered;

    case QInAppTransaction::PurchaseApproved:
      logger.log() << "Purchase approved";
      purchaseCompleted();
      break;

    case QInAppTransaction::PurchaseRestored:
      logger.log() << "Purchase Restored";
      purchaseCompleted();
      break;

    case QInAppTransaction::Unknown:
    default:
      logger.log() << "unexpected transaction state";
      emit subscriptionFailed();
      break;
            }

            m_started = false;
            transaction->finalize();
});
}

IAPHandler::~IAPHandler() {
  MVPN_COUNT_DTOR(IAPHandler);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
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
  m_appStore.registerProduct(QInAppProduct::Consumable, m_productName);

  logger.log() << "Waiting for the products registration";
}

void IAPHandler::startSubscription(bool restore) {
  Q_ASSERT(m_productsRegistrationState == eRegistered);
  Q_ASSERT(m_productName.isEmpty());

  if (m_started) {
    logger.log() << "No multiple IAP!";
    return;
  }

  m_started = true;

  if (restore) {
    logger.log() << "Restore the subscription";
    m_appStore.restorePurchases();
    return;
  }

  logger.log() << "Starting the subscription";

  QInAppProduct* product = m_appStore.registeredProduct(m_productName);
  Q_ASSERT(product);

  product->purchase();
}

void IAPHandler::purchaseCompleted() {
  logger.log() << "Purchase completed";

  QString receipt = IOSUtils::IAPReceipt();
  if (receipt.isEmpty()) {
    logger.log() << "Empty receipt found";
    emit subscriptionFailed();
    return;
  }

  NetworkRequest* request = NetworkRequest::createForIOSPurchase(this, receipt);

  connect(
      request, &NetworkRequest::requestFailed,
      [this](QNetworkReply::NetworkError error) {
        logger.log() << "Purchase request failed" << error;
        MozillaVPN::instance()->errorHandle(ErrorHandler::toErrorType(error));
        emit subscriptionFailed();
      });

  connect(request, &NetworkRequest::requestCompleted,
          [this](const QByteArray&) {
            logger.log() << "Purchase request completed";
            emit subscriptionCompleted();
          });
}
