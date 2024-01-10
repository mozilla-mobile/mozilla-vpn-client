/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskheartbeat.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "context/constants.h"
#include "logging/logger.h"
#include "mozillavpn.h"
#include "networking/networkrequest.h"
#include "utilities/leakdetector.h"

namespace {
Logger logger("TaskHeartbeat");
}

TaskHeartbeat::TaskHeartbeat() : Task("TaskHeartbeat") {
  MZ_COUNT_CTOR(TaskHeartbeat);
}

TaskHeartbeat::~TaskHeartbeat() { MZ_COUNT_DTOR(TaskHeartbeat); }

void TaskHeartbeat::run() {
  NetworkRequest* request = new NetworkRequest(this, 200);
  request->get(Constants::apiUrl(Constants::Heartbeat));

  connect(request, &NetworkRequest::requestFailed, this,
          [this, request](QNetworkReply::NetworkError, const QByteArray&) {
            int statusCode = request->statusCode();
            logger.error() << "Failed to talk with the server. Status code: "
                           << statusCode;

            MozillaVPN* vpn = MozillaVPN::instance();
            Q_ASSERT(vpn);

            // Internal server errors.
            if (statusCode >= 500 && statusCode <= 509) {
              vpn->heartbeatCompleted(false);
            }

            // Client errors.
            else if (statusCode >= 400 && statusCode <= 409) {
              vpn->heartbeatCompleted(false);
            }

            // We don't know if this happeneded because of a global network
            // failure or a local network issue. In general, let's ignore this
            // error.
            else {
              vpn->heartbeatCompleted(true);
            }

            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Heartbeat content received:" << data;

            MozillaVPN* vpn = MozillaVPN::instance();
            Q_ASSERT(vpn);

            QJsonObject json = QJsonDocument::fromJson(data).object();
            QJsonValue mullvad = json.value("mullvadOK");
            QJsonValue db = json.value("dbOK");

            vpn->heartbeatCompleted(!mullvad.isBool() || !db.isBool() ||
                                    (mullvad.toBool() && db.toBool()));

            emit completed();
          });
}
