/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskproducts.h"

#include "app.h"
#include "appconstants.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "productshandler.h"

namespace {
Logger logger("TaskProducts");
}

TaskProducts::TaskProducts() : Task("TaskProducts") {
  MZ_COUNT_CTOR(TaskProducts);
}

TaskProducts::~TaskProducts() { MZ_COUNT_DTOR(TaskProducts); }

void TaskProducts::run() {
  NetworkRequest* request = new NetworkRequest(this, 200);
  request->auth(App::authorizationHeader());
  request->get(AppConstants::apiUrl(AppConstants::Products));

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
