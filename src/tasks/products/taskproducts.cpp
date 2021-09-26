/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskproducts.h"
#include "core.h"
#include "iaphandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

namespace {
Logger logger(LOG_IAP, "TaskProducts");
}

TaskProducts::TaskProducts() : Task("TaskProducts") {
  MVPN_COUNT_CTOR(TaskProducts);
}

TaskProducts::~TaskProducts() { MVPN_COUNT_DTOR(TaskProducts); }

void TaskProducts::run(Core* core) {
  NetworkRequest* request = NetworkRequest::createForProducts(this);

  connect(request, &NetworkRequest::requestFailed,
          [this, core](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Products request to guardian failed" << error;
            core->errorHandle(ErrorHandler::toErrorType(error));
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this](const QByteArray& data) {
            logger.debug() << "Products request to guardian completed" << data;

            IAPHandler* iapHandler = IAPHandler::instance();
            Q_ASSERT(iapHandler);

            connect(iapHandler, &IAPHandler::productsRegistered, this,
                    &TaskProducts::completed);
            connect(iapHandler, &IAPHandler::productsRegistrationStopped, this,
                    &TaskProducts::completed);
            iapHandler->registerProducts(data);
          });
}
