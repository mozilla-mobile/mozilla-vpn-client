/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskiosproducts.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

void TaskIOSProducts::run(MozillaVPN* vpn)
{
    NetworkRequest *request = NetworkRequest::createForIOSProducts(vpn);

    connect(request, &NetworkRequest::requestFailed, [this, vpn](QNetworkReply::NetworkError error) {
        qDebug() << "IOS product request failed" << error;
        vpn->errorHandle(ErrorHandler::toErrorType(error));
        emit completed();
    });

    connect(request, &NetworkRequest::requestCompleted, [this, vpn](const QByteArray &data) {
        qDebug() << "IOS product request completed" << data;

        QJsonDocument json = QJsonDocument::fromJson(data);
        Q_ASSERT(json.isObject());

        QJsonObject obj = json.object();
        Q_ASSERT(obj.contains("products"));

        QJsonValue productsValue = obj.take("products");
        Q_ASSERT(productsValue.isArray());

        QJsonArray productsArray = productsValue.toArray();

        QStringList products;
        for (QJsonArray::ConstIterator i = productsArray.begin(); i != productsArray.end(); ++i) {
            Q_ASSERT(i->isString());
            products.append(i->toString());
        }

        vpn->settingsHolder()->setIapProducts(products);
        emit completed();
    });
}
