/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/ios/iaphandler.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "settingsholder.h"

#include <QInAppStore>
#include <QtPurchasing>

namespace {
Logger logger(LOG_IAP, "IAPHandler");
}

void IAPHandler::start()
{
    logger.log() << "Starting the subscription";

    Q_ASSERT(!m_appStore);
    m_appStore = new QInAppStore(this);

    connect(m_appStore, &QInAppStore::productRegistered, [](QInAppProduct *product) {
        logger.log() << "Product registered";
        logger.log() << "Title:" << product->title();
        logger.log() << "Description:" << product->description();
        logger.log() << "Price:" << product->price();

        product->purchase();
    });

    connect(m_appStore,
            &QInAppStore::productUnknown,
            [this](QInAppProduct::ProductType productType, const QString &identifier) {
                logger.log() << "Product registration failed:" << productType << identifier;
                emit failed();
            });

    connect(m_appStore, &QInAppStore::transactionReady, [this](QInAppTransaction *transaction) {
        logger.log() << "Transaction ready - status:" << transaction->status();

        switch (transaction->status()) {
        case QInAppTransaction::PurchaseFailed:
            logger.log() << "Purchase Failed" << transaction->errorString()
                         << "Reason:" << transaction->failureReason();
            emit failed();
            break;

        case QInAppTransaction::PurchaseApproved:
            logger.log() << "Purchase approved";
            purchaseCompleted(transaction->orderId());
            break;

        case QInAppTransaction::PurchaseRestored:
            logger.log() << "Purchase Restored";
            break;

        case QInAppTransaction::Unknown:
        default:
            logger.log() << "unexpected transaction state";
            emit failed();
            break;
        }

        transaction->finalize();
    });

    const QStringList products = SettingsHolder::instance()->iapProducts();
    Q_ASSERT(!products.isEmpty());

    for (const QString &product : products) {
        logger.log() << "Registration product:" << product;
        m_appStore->registerProduct(QInAppProduct::Consumable, product);
    }

    logger.log() << "Waiting for the registreation of products";
}

void IAPHandler::purchaseCompleted(const QString& orderId)
{
    NetworkRequest *request = NetworkRequest::createForIOSPurchase(this, orderId);

    connect(request,
            &NetworkRequest::requestFailed,
            [this](QNetworkReply::NetworkError error) {
                logger.log() << "Purchase request failed" << error;
                MozillaVPN::instance()->errorHandle(ErrorHandler::toErrorType(error));
                emit completed();
            });

    connect(request, &NetworkRequest::requestCompleted, [this](const QByteArray &) {
        logger.log() << "Purchase request completed";
        emit completed();
    });
}
