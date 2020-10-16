/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "platforms/ios/iaphandler.h"
#include "mozillavpn.h"

#include <QDebug>
#include <QInAppStore>
#include <QtPurchasing>

void IAPHandler::start()
{
    qDebug() << "Starting the subscription";

    Q_ASSERT(!m_appStore);
    m_appStore = new QInAppStore(this);

    connect(m_appStore, &QInAppStore::productRegistered, [](QInAppProduct *product) {
        qDebug() << "Product registered" << product;
        qDebug() << "Title:" << product->title();
        qDebug() << "Description:" << product->description();
        qDebug() << "Price:" << product->price();

        product->purchase();
    });

    connect(m_appStore, &QInAppStore::productUnknown, [this](QInAppProduct::ProductType productType, const QString &identifier) {
        qDebug() << "Product registration failed:" << productType << identifier;
        emit failed();
    });

    connect(m_appStore, &QInAppStore::transactionReady, [this](QInAppTransaction *transaction) {
        qDebug() << "Transaction ready" << transaction << "status:" << transaction->status();

        switch (transaction->status()) {
            case QInAppTransaction::PurchaseFailed:
                qDebug() << "Purchase Failed" << transaction->errorString() << "Reason:" << transaction->failureReason();
                emit failed();
                break;

            case QInAppTransaction::PurchaseApproved:
                qDebug() << "Purchase approved";
                emit completed();
                break;

            case QInAppTransaction::PurchaseRestored:
                qDebug() << "Purchase Restored";
                break;

            case QInAppTransaction::Unknown:
            default:
                qDebug() << "unexpected transaction state";
                emit failed();
                break;
        }

        transaction->finalize();
    });

    const QStringList products = MozillaVPN::instance()->settingsHolder()->iapProducts();
    Q_ASSERT(!products.isEmpty());

    for (QStringList::ConstIterator i = products.begin(); i != products.end(); ++i) {
        qDebug() << "Registration product:" << *i;
        m_appStore->registerProduct(QInAppProduct::Consumable, *i);
    }

    qDebug() << "Waiting for the registreation of products";
}
