/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskgetfeaturelist.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "appconstants.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/featuremodel.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskGetFeatureList");
}

TaskGetFeatureList::TaskGetFeatureList() : Task("TaskGetFeatureList") {
  MZ_COUNT_CTOR(TaskGetFeatureList);
}

TaskGetFeatureList::~TaskGetFeatureList() { MZ_COUNT_DTOR(TaskGetFeatureList); }

void TaskGetFeatureList::run() {
  NetworkRequest* request = new NetworkRequest(this, 200);
  request->get(AppConstants::apiUrl(AppConstants::FeatureList));

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Get feature list is failed" << error;
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Get feature list is completed" << data;
            FeatureModel::instance()->updateFeatureList(data);
            emit completed();
          });
}
