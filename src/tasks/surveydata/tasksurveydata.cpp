/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tasksurveydata.h"
#include "core.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

namespace {
Logger logger(LOG_MAIN, "TaskSurveyData");
}

TaskSurveyData::TaskSurveyData() : Task("TaskSurveyData") {
  MVPN_COUNT_CTOR(TaskSurveyData);
}

TaskSurveyData::~TaskSurveyData() { MVPN_COUNT_DTOR(TaskSurveyData); }

void TaskSurveyData::run(Core* core) {
  logger.debug() << "Fetch survey data";

  NetworkRequest* request = NetworkRequest::createForSurveyData(this);

  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Failed to fetch survey data" << error;
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this, core](const QByteArray& data) {
            logger.debug() << "Survey data fetched";
            core->surveyChecked(data);
            emit completed();
          });
}
