/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskaccountandservers.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/servercountrymodel.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger(LOG_MAIN, "TaskAccountAndServers");
}

TaskAccountAndServers::TaskAccountAndServers() : Task("TaskAccountAndServers") {
  MVPN_COUNT_CTOR(TaskAccountAndServers);
}

TaskAccountAndServers::~TaskAccountAndServers() {
  MVPN_COUNT_DTOR(TaskAccountAndServers);
}

void TaskAccountAndServers::run(MozillaVPN* vpn) {
  // Account fetch and servers fetch run in parallel.

  // Account fetch
  {
    NetworkRequest* request = NetworkRequest::createForAccount(this);

    connect(
        request, &NetworkRequest::requestFailed,
        [this, vpn](QNetworkReply::NetworkError error, const QByteArray&, int) {
          logger.error() << "Account request failed" << error;
          vpn->errorHandle(ErrorHandler::toErrorType(error));
          m_accountCompleted = true;
          maybeCompleted();
        });

    connect(request, &NetworkRequest::requestCompleted,
            [this, vpn](const QByteArray& data) {
              logger.error() << "Account request completed";
              vpn->accountChecked(data);
              m_accountCompleted = true;
              maybeCompleted();
            });
  }

  // Server list fetch
  {
    NetworkRequest* request = NetworkRequest::createForServers(this);

    connect(
        request, &NetworkRequest::requestFailed,
        [this, vpn](QNetworkReply::NetworkError error, const QByteArray&, int) {
          logger.error() << "Failed to retrieve servers";
          vpn->errorHandle(ErrorHandler::toErrorType(error));
          m_serversCompleted = true;
          maybeCompleted();
        });

    connect(request, &NetworkRequest::requestCompleted,
            [this, vpn](const QByteArray& data) {
              logger.debug() << "Servers obtained";
              vpn->serversFetched(data);
              m_serversCompleted = true;
              maybeCompleted();
            });
  }
}

void TaskAccountAndServers::maybeCompleted() {
  if (m_accountCompleted && m_serversCompleted) {
    emit completed();
  }
}
