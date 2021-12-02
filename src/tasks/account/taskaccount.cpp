/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskaccount.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger(LOG_MAIN, "TaskAccount");
}

TaskAccount::TaskAccount() : Task("TaskAccount") {
  MVPN_COUNT_CTOR(TaskAccount);
}

TaskAccount::~TaskAccount() { MVPN_COUNT_DTOR(TaskAccount); }

void TaskAccount::run() {
  NetworkRequest* request = NetworkRequest::createForAccount(this);

  connect(
      request, &NetworkRequest::requestFailed, this,
      [this](QNetworkReply::NetworkError error, const QByteArray&) {
        logger.error() << "Account request failed" << error;
        MozillaVPN::instance().errorHandle(ErrorHandler::toErrorType(error));
        emit completed();
      });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Account request completed";
            MozillaVPN::instance().accountChecked(data);
            emit completed();
          });
}
