/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tasksurveydata.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger(LOG_MAIN, "TaskSurveyData");
}

TaskSurveyData::TaskSurveyData() : Task("TaskSurveyData") {
  MVPN_COUNT_CTOR(TaskSurveyData);
}

TaskSurveyData::~TaskSurveyData() { MVPN_COUNT_DTOR(TaskSurveyData); }

void TaskSurveyData::run(MozillaVPN* vpn) {
  logger.log() << "Fetch survey data";

  NetworkRequest* request = NetworkRequest::createForSurveyData(this);

  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.log() << "Failed to fetch survey data" << error;
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this, vpn](const QByteArray& data) {
            logger.log() << "Survey data fetched";
            vpn->surveyChecked(data);
            emit completed();
          });
}
