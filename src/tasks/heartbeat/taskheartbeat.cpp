/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskheartbeat.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskHeartbeat");
}

TaskHeartbeat::TaskHeartbeat() : Task("TaskHeartbeat") {
  MZ_COUNT_CTOR(TaskHeartbeat);
}

TaskHeartbeat::~TaskHeartbeat() { MZ_COUNT_DTOR(TaskHeartbeat); }

void TaskHeartbeat::run() {
  NetworkRequest* request = new NetworkRequest(this, 200);
  request->get(AppConstants::apiUrl(AppConstants::Heartbeat));

  connect(request, &NetworkRequest::requestFailed, this,
          [this, request](QNetworkReply::NetworkError, const QByteArray&) {
            logger.error() << "Failed to talk with the server";

            MozillaVPN* vpn = MozillaVPN::instance();
            Q_ASSERT(vpn);

            int statusCode = request->statusCode();

            // Internal server errors.
            if (statusCode >= 500 && statusCode <= 509) {
              vpn->heartbeatCompleted(false);
            }

            // Request failure ((?!?)
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
