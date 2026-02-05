/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskauthenticate.h"

#include <QJSValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>

#include "authenticationlistener.h"
#include "constants.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "settingsholder.h"

namespace {
Logger logger("TaskAuthenticate");
}  // anonymous namespace

TaskAuthenticate::TaskAuthenticate(
    AuthenticationListener::AuthenticationType authenticationType)
    : Task("TaskAuthenticate"), m_authenticationType(authenticationType) {
  MZ_COUNT_CTOR(TaskAuthenticate);
}

TaskAuthenticate::~TaskAuthenticate() { MZ_COUNT_DTOR(TaskAuthenticate); }

void TaskAuthenticate::run() {
  logger.debug() << "TaskAuthenticate::Run";

  Q_ASSERT(!m_authenticationListener);

  QByteArray challenge;
  AuthenticationListener::generatePkceCodes(m_pkceCodeVerifier, challenge);
  Q_ASSERT(!m_pkceCodeVerifier.isEmpty() && !challenge.isEmpty());

  m_authenticationListener =
      AuthenticationListener::create(this, m_authenticationType);

  connect(m_authenticationListener, &AuthenticationListener::readyToFinish,
          this, &Task::completed);

  connect(m_authenticationListener, &AuthenticationListener::completed, this,
          &TaskAuthenticate::authenticatePkceSuccess);

  connect(m_authenticationListener, &AuthenticationListener::failed, this,
          [this](ErrorHandler::ErrorType error) {
            REPORTERROR(error, name());
            m_authenticationListener->aboutToFinish();
          });

  connect(m_authenticationListener, &AuthenticationListener::abortedByUser,
          this, [this]() {
            emit authenticationAborted();
            m_authenticationListener->aboutToFinish();
          });

  connect(m_authenticationListener, &AuthenticationListener::started, this,
          [this]() { emit authenticationStarted(); });

  m_metricUuid = QUuid::createUuid();
  m_authenticationListener->start(this, challenge, CODE_CHALLENGE_METHOD,
                                  SettingsHolder::instance()->userEmail());
}

void TaskAuthenticate::authenticatePkceSuccess(const QString& code) {
  logger.debug() << "Authentication completed with code:"
                 << logger.sensitive(code);

  NetworkRequest* request = new NetworkRequest(this, 200);
  QJsonObject payload;
  payload.insert("code", code);
  payload.insert("code_verifier", QString(m_pkceCodeVerifier));
  request->post(AuthenticationListener::createLoginVerifyUrl(), payload);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Failed to complete the authentication" << error;
            REPORTNETWORKERROR(error, ErrorHandler::PropagateError, name());
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          &TaskAuthenticate::authenticationCompletedInternal);
}

void TaskAuthenticate::authenticationCompletedInternal(const QByteArray& data) {
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

  emit authenticationCompleted(userDoc.toJson(QJsonDocument::Compact),
                               tokenValue.toString());

  m_authenticationListener->aboutToFinish();
}

void TaskAuthenticate::handleDeepLink(const QUrl& url) {
  if ((url.scheme() != Constants::DEEP_LINK_SCHEME) ||
      (url.authority() != "login")) {
    return;
  }

  if (url.path() != "/success") {
    logger.warning() << "Received unexpected auth endpoint:" << url.path();
    return;
  }

  QUrlQuery query(url.query());
  if (!query.hasQueryItem("code")) {
    logger.warning() << "Received OAuth success, but no code was found";
    return;
  }

  authenticatePkceSuccess(query.queryItemValue("code"));
}