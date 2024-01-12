/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskproducts.h"

#include "context/app.h"
#include "context/constants.h"
#include "errorhandler.h"
#include "logging/logger.h"
#include "networking/networkrequest.h"
#include "productshandler.h"
#include "utilities/leakdetector.h"

namespace {
Logger logger("TaskProducts");
}

TaskProducts::TaskProducts() : Task("TaskProducts") {
  MZ_COUNT_CTOR(TaskProducts);
}

TaskProducts::~TaskProducts() { MZ_COUNT_DTOR(TaskProducts); }

void TaskProducts::run() {
  NetworkRequest* request = new NetworkRequest(this, 200);
  request->get(QString("%1/v1/oauth/subscriptions/iap/plans/%2")
                   .arg(Constants::fxaApiBaseUrl(), Constants::IAP_PLANS));

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Products request to guardian failed" << error;
            REPORTNETWORKERROR(error, ErrorHandler::PropagateError, name());
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
