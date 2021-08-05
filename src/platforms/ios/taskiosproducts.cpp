/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskiosproducts.h"
#include "iosiaphandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

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
            logger.error() << "IOS product request failed" << error;
            vpn->errorHandle(ErrorHandler::toErrorType(error));
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this](const QByteArray& data) {
            logger.debug() << "IOS product request completed" << data;

            IOSIAPHandler* ipaHandler = IOSIAPHandler::instance();
            Q_ASSERT(ipaHandler);

            connect(ipaHandler, &IOSIAPHandler::productsRegistered, this,
                    &TaskIOSProducts::completed);
            ipaHandler->registerProducts(data);
          });
}
