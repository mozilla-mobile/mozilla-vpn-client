#include "platforms/ios/iaphandler.h"

#include <QDebug>
#include <QInAppStore>
#include <QtPurchasing>

constexpr const char *PRODUCT_ID = "org.mozilla.ios.FirefoxVPN_staging.product.test.1m_renew_subscription";

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

    connect(m_appStore, &QInAppStore::productUnknown, [](QInAppProduct::ProductType productType, const QString &identifier) {
        qDebug() << "Product registration failed:" << productType << identifier;
        // TODO
    });

    connect(m_appStore, &QInAppStore::transactionReady, [](QInAppTransaction *transaction) {
        qDebug() << "Transaction ready" << transaction << "status:" << transaction->status();

        switch (transaction->status()) {
            case QInAppTransaction::PurchaseFailed:
                qDebug() << "Purchase Failed" << transaction->errorString() << "Reason:" << transaction->failureReason();
                break;

            case QInAppTransaction::PurchaseApproved:
                qDebug() << "Purchase approved";
                break;

            case QInAppTransaction::PurchaseRestored:
                qDebug() << "Purchase Restored";
                break;

            case QInAppTransaction::Unknown:
            default:
                qDebug() << "unexpected transaction state";
                // TODO
                break;
        }

        transaction->finalize();
    });

    m_appStore->registerProduct(QInAppProduct::Consumable, PRODUCT_ID);
    qDebug() << "Waiting for the registreation of products";
}
