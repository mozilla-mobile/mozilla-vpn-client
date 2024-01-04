/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskgetlocation.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "constants.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/location.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskGetLocation");
}

TaskGetLocation::TaskGetLocation(
    ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy)
    : Task("TaskGetLocation"),
      m_errorPropagationPolicy(errorPropagationPolicy) {
  MZ_COUNT_CTOR(TaskGetLocation);
}

TaskGetLocation::~TaskGetLocation() { MZ_COUNT_DTOR(TaskGetLocation); }

void TaskGetLocation::run() {
  QUrl url(Constants::apiUrl(Constants::IPInfo));
  QString host = url.host();

  NetworkRequest* request = new NetworkRequest(this, 200);
  request->auth();
  request->requestInternal().setRawHeader("Host", host.toLocal8Bit());
  request->requestInternal().setPeerVerifyName(host);
  request->get(url);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Failed to retrieve client location";
            REPORTNETWORKERROR(error, m_errorPropagationPolicy, name());
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            if (!MozillaVPN::instance()->location()->fromJson(data)) {
              logger.warning() << "Failed to parse client location";
            }
            emit completed();
          });
}
