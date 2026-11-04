/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tasktoken.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "constants.h"
#include "controller.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskToken");
}

TaskToken::TaskToken(
    ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy)
    : Task("TaskToken"), m_errorPropagationPolicy(errorPropagationPolicy) {
  MZ_COUNT_CTOR(TaskToken);
}

TaskToken::~TaskToken() { MZ_COUNT_DTOR(TaskToken); }

void TaskToken::run() {
  // Fetch a MASQUE token from Guardian, using the FxA token from the
  // environment. The token comes back in the "token" field.
  QByteArray fxToken = qgetenv("MASQUE_FX_TOKEN");
  if (fxToken.isEmpty()) {
    logger.error() << "MASQUE_FX_TOKEN is not set; cannot fetch a MASQUE token";
    emit completed();
    return;
  }

  // QUrl url(Constants::apiBaseUrl() + "/api/v1/fpn/token");
  QUrl url("https://vpn.mozilla.org/api/v1/fpn/token");

  NetworkRequest* request = new NetworkRequest(this, 200);
  request->auth("Bearer " + fxToken);
  request->get(url);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Failed to retrieve MASQUE token";
            REPORTNETWORKERROR(error, m_errorPropagationPolicy, name());
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (!doc.isObject()) {
              logger.error() << "Invalid JSON in MASQUE token response";
              emit completed();
              return;
            }
            QString token = doc.object().value("token").toString();
            if (token.isEmpty()) {
              logger.error() << "MASQUE token response is missing 'token'";
              emit completed();
              return;
            }

            MozillaVPN* vpn = MozillaVPN::instance();
            vpn->setToken(token);

            // Push the new token to a running tunnel. No-op if not active.
            vpn->controller()->rotateToken(token);

            logger.debug() << "MASQUE token refreshed";
            emit completed();
          });
}