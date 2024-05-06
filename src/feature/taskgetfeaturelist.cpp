/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskgetfeaturelist.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

#include "app.h"
#include "constants.h"
#include "feature/featuremodel.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "settingsholder.h"

namespace {
Logger logger("TaskGetFeatureList");
}

TaskGetFeatureList::TaskGetFeatureList() : Task("TaskGetFeatureList") {
  MZ_COUNT_CTOR(TaskGetFeatureList);
}

TaskGetFeatureList::~TaskGetFeatureList() { MZ_COUNT_DTOR(TaskGetFeatureList); }

void TaskGetFeatureList::run() {
  NetworkRequest* request = new NetworkRequest(this, 200);

  QJsonObject body;
  if (SettingsHolder::instance()->token().isEmpty()) {
    auto unauthedExperimenterId =
        SettingsHolder::instance()->unauthedExperimenterId();

    if (unauthedExperimenterId.isEmpty()) {
      unauthedExperimenterId = QUuid::createUuid().toString();
      SettingsHolder::instance()->setUnauthedExperimenterId(
          unauthedExperimenterId);
    }

    body["experimenterId"] = unauthedExperimenterId;
  } else {
    request->auth();
  }

  request->post(Constants::apiUrl(Constants::FeatureList), body);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Get feature list has failed" << error;
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Get feature list completed" << data;
            FeatureModel::instance()->updateFeatureList(data);
            emit completed();
          });
}
