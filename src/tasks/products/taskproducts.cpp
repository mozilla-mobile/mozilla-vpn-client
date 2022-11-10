/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskproducts.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "productshandler.h"

namespace {
Logger logger(LOG_IAP, "TaskProducts");
}

TaskProducts::TaskProducts() : Task("TaskProducts") {
  MVPN_COUNT_CTOR(TaskProducts);
}

TaskProducts::~TaskProducts() { MVPN_COUNT_DTOR(TaskProducts); }

void TaskProducts::run() {
  NetworkRequest* request = NetworkRequest::createForProducts(this);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Products request to guardian failed" << error;
            ErrorHandler::networkErrorHandle(error);
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Products request to guardian completed" << data;

            ProductsHandler* productsHandler = ProductsHandler::instance();
            Q_ASSERT(productsHandler);

            connect(productsHandler, &ProductsHandler::productsRegistered, this,
                    &TaskProducts::completed);
            connect(productsHandler,
                    &ProductsHandler::productsRegistrationStopped, this,
                    &TaskProducts::completed);
            productsHandler->registerProducts(data);
          });
}
