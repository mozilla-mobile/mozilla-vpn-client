/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskgetfeaturelist.h"
#include "leakdetector.h"
#include "errorhandler.h"
#include "logger.h"
#include "featurelist.h"
#include "networkrequest.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace {
Logger logger(LOG_MAIN, "TaskGetFeatureList");
}

TaskGetFeatureList::TaskGetFeatureList() : Task("TaskGetFeatureList") {
  MVPN_COUNT_CTOR(TaskGetFeatureList);
}

TaskGetFeatureList::~TaskGetFeatureList() {
  MVPN_COUNT_DTOR(TaskGetFeatureList);
}

void TaskGetFeatureList::run() {
  NetworkRequest* request = NetworkRequest::createForGetFeatureList(this);

  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Get feature list is failed" << error;
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this](const QByteArray& data) {
            logger.debug() << "Get feature list is completed" << data;
            FeatureList::instance().updateFeatureList(data);
            emit completed();
          });
}
