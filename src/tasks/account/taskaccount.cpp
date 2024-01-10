/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskaccount.h"

#include "app.h"
#include "context/constants.h"
#include "logging/logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "utilities/leakdetector.h"

namespace {
Logger logger("TaskAccount");
}

TaskAccount::TaskAccount(
    ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy)
    : Task("TaskAccount"), m_errorPropagationPolicy(errorPropagationPolicy) {
  MZ_COUNT_CTOR(TaskAccount);
}

TaskAccount::~TaskAccount() { MZ_COUNT_DTOR(TaskAccount); }

void TaskAccount::run() {
  NetworkRequest* request = new NetworkRequest(this, 200);
  request->auth(App::authorizationHeader());
  request->get(Constants::apiUrl(Constants::Account));

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Account request failed" << error;
            REPORTNETWORKERROR(error, m_errorPropagationPolicy, name());
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Account request completed";
            MozillaVPN::instance()->accountChecked(data);
            emit completed();
          });
}
