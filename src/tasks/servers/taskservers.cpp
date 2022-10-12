/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskservers.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger(LOG_MAIN, "TaskServers");
}

TaskServers::TaskServers(
    ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy)
    : Task("TaskServers"), m_errorPropagationPolicy(errorPropagationPolicy) {
  MVPN_COUNT_CTOR(TaskServers);
}

TaskServers::~TaskServers() { MVPN_COUNT_DTOR(TaskServers); }

void TaskServers::run() {
  NetworkRequest* request = NetworkRequest::createForServers(this);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Failed to retrieve servers";
            ErrorHandler::networkErrorHandle(error, m_errorPropagationPolicy);
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Servers obtained";
            MozillaVPN::instance()->serversFetched(data);
            emit completed();
          });
}
