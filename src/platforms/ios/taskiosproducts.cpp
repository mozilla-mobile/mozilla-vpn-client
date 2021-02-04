/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskiosproducts.h"
#include "iaphandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "settingsholder.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace {
Logger logger(LOG_IAP, "TaskIOSProducts");
}

TaskIOSProducts::TaskIOSProducts() : Task("TaskIOSProducts") {
  MVPN_COUNT_CTOR(TaskIOSProducts);
}

TaskIOSProducts::~TaskIOSProducts() { MVPN_COUNT_DTOR(TaskIOSProducts); }

void TaskIOSProducts::run(MozillaVPN* vpn) {
  NetworkRequest* request = NetworkRequest::createForIOSProducts(this);

  connect(request, &NetworkRequest::requestFailed,
          [this, vpn](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.log() << "IOS product request failed" << error;
            vpn->errorHandle(ErrorHandler::toErrorType(error));
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this](QNetworkReply*, const QByteArray& data) {
            logger.log() << "IOS product request completed" << data;

            QJsonDocument json = QJsonDocument::fromJson(data);
            Q_ASSERT(json.isObject());

            QJsonObject obj = json.object();
            Q_ASSERT(obj.contains("products"));

            QJsonValue productsValue = obj.value("products");
            Q_ASSERT(productsValue.isArray());

            QJsonArray productsArray = productsValue.toArray();

            QStringList products;
            for (QJsonValue product : productsArray) {
              Q_ASSERT(product.isString());
              products.append(product.toString());
            }

            SettingsHolder::instance()->setIapProducts(products);

            IAPHandler* ipaHandler = IAPHandler::instance();
            Q_ASSERT(ipaHandler);

            connect(ipaHandler, &IAPHandler::productsRegistered, this,
                    &TaskIOSProducts::completed);
            ipaHandler->registerProducts(products);
          });
}
