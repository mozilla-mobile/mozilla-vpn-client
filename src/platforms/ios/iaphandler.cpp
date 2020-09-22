#include "iaphandler.h"

#include <QDebug>
#include <QInAppStore>
#include <QtPurchasing>

void IAPHandler::start()
{
    qDebug() << "Starting the subscription";

    Q_ASSERT(!m_appStore);
    m_appStore = new QInAppStore(this);

    connect(m_appStore, &QInAppStore::productRegistered, [](QInAppProduct *iap) {
        qDebug() << "Product registered" << iap;
    });

    connect(m_appStore, &QInAppStore::productUnknown, [](QInAppProduct::ProductType productType, const QString &identifier) {
        qDebug() << "Product registration failed:" << productType << identifier;
    });

    connect(m_appStore, &QInAppStore::transactionReady, [](QInAppTransaction *transaction) {
        qDebug() << "Transaction ready" << transaction;
    });

    m_appStore->registerProduct(QInAppProduct::Unlockable, "Mozilla VPN");
    qDebug() << "Waiting for the registreation of products";
}
