/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskauthenticate.h"

#include <QJSValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>

#include "appconstants.h"
#include "authenticationlistener.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/user.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "settingsholder.h"

namespace {
Logger logger("TaskAuthenticate");
}  // anonymous namespace

TaskAuthenticate::TaskAuthenticate(
    MozillaVPN::AuthenticationType authenticationType)
    : Task("TaskAuthenticate"), m_authenticationType(authenticationType) {
  MZ_COUNT_CTOR(TaskAuthenticate);
}

TaskAuthenticate::~TaskAuthenticate() { MZ_COUNT_DTOR(TaskAuthenticate); }

void TaskAuthenticate::run() {
  logger.debug() << "TaskAuthenticate::Run";

  Q_ASSERT(!m_authenticationListener);

  QByteArray pkceCodeVerifier;
  QByteArray pkceCodeChallenge;
  AuthenticationListener::generatePkceCodes(pkceCodeVerifier,
                                            pkceCodeChallenge);
  Q_ASSERT(!pkceCodeVerifier.isEmpty() && !pkceCodeChallenge.isEmpty());

  m_authenticationListener =
      AuthenticationListener::create(this, m_authenticationType);

  connect(m_authenticationListener, &AuthenticationListener::readyToFinish,
          this, &Task::completed);

  connect(m_authenticationListener, &AuthenticationListener::completed, this,
          [this, pkceCodeVerifier](const QString& pkceCodeSuccess) {
            logger.debug() << "Authentication completed with code:"
                           << logger.sensitive(pkceCodeSuccess);

            NetworkRequest* request = new NetworkRequest(this, 200);
            request->post(
                AppConstants::apiUrl(AppConstants::LoginVerify),
                QJsonObject{{"code", pkceCodeSuccess},
                            {"code_verifier", QString(pkceCodeVerifier)}});

            connect(
                request, &NetworkRequest::requestFailed, this,
                [this](QNetworkReply::NetworkError error, const QByteArray&) {
                  logger.error()
                      << "Failed to complete the authentication" << error;
                  REPORTNETWORKERROR(error, ErrorHandler::PropagateError,
                                     name());
                });

            connect(request, &NetworkRequest::requestCompleted, this,
                    [this](const QByteArray& data) {
                      logger.debug() << "Authentication completed";
                      authenticationCompleted(data);
                    });
          });

  connect(m_authenticationListener, &AuthenticationListener::failed, this,
          [this](const ErrorHandler::ErrorType error) {
            REPORTERROR(error, name());
            m_authenticationListener->aboutToFinish();
          });

  connect(m_authenticationListener, &AuthenticationListener::abortedByUser,
          this, [this]() {
            MozillaVPN::instance()->abortAuthentication();
            m_authenticationListener->aboutToFinish();
          });

  m_authenticationListener->start(this, pkceCodeChallenge,
                                  CODE_CHALLENGE_METHOD,
                                  SettingsHolder::instance()->userEmail());
}

void TaskAuthenticate::authenticationCompleted(const QByteArray& data) {
  logger.debug() << "Authentication completed";

  QJsonDocument json = QJsonDocument::fromJson(data);
  if (json.isNull()) {
    REPORTERROR(ErrorHandler::RemoteServiceError, name());
    return;
  }

  QJsonObject obj = json.object();
  QJsonValue userObj = obj.value("user");
  if (!userObj.isObject()) {
    REPORTERROR(ErrorHandler::RemoteServiceError, name());
    return;
  }

  logger.debug() << "User data:"
                 << logger.sensitive(QJsonDocument(userObj.toObject())
                                         .toJson(QJsonDocument::Compact));

  QJsonValue tokenValue = obj.value("token");
  if (!tokenValue.isString()) {
    REPORTERROR(ErrorHandler::RemoteServiceError, name());
    return;
  }

  QJsonDocument userDoc;
  userDoc.setObject(userObj.toObject());

  MozillaVPN::instance()->authenticationCompleted(
      userDoc.toJson(QJsonDocument::Compact), tokenValue.toString());

  m_authenticationListener->aboutToFinish();
}
