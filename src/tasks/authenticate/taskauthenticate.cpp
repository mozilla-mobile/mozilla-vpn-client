/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskauthenticate.h"

#include <QDateTime>
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
#include "settings/settingsmanager.h"
#include "settingsholder.h"

// Expiration timeout for persisted PKCE code verifier
constexpr int PENDING_AUTH_MAX_AGE_SECS = 15 * 60;

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

  m_authenticationListener =
      AuthenticationListener::create(this, m_authenticationType);

  connect(m_authenticationListener, &AuthenticationListener::readyToFinish,
          this, &Task::completed);

  connect(m_authenticationListener, &AuthenticationListener::completed, this,
          &TaskAuthenticate::authenticatePkceSuccess);

  connect(m_authenticationListener, &AuthenticationListener::failed, this,
          [this](ErrorHandler::ErrorType error) {
            clearPendingCodeVerifier();
            REPORTERROR(error, name());
            m_authenticationListener->aboutToFinish();
          });

  connect(m_authenticationListener, &AuthenticationListener::abortedByUser,
          this, [this]() {
            clearPendingCodeVerifier();
            emit authenticationAborted();
            m_authenticationListener->aboutToFinish();
          });

  connect(m_authenticationListener, &AuthenticationListener::started, this,
          [this]() { emit authenticationStarted(); });

  if (!m_resumeCode.isEmpty()) {
    logger.debug() << "Resuming a previously started authentication";
    Q_ASSERT(!m_pkceCodeVerifier.isEmpty());
    authenticatePkceSuccess(m_resumeCode);
    return;
  }

  QByteArray challenge;
  AuthenticationListener::generatePkceCodes(m_pkceCodeVerifier, challenge);
  Q_ASSERT(!m_pkceCodeVerifier.isEmpty() && !challenge.isEmpty());

#ifdef MZ_ANDROID
  if (m_authenticationType == AuthenticationListener::AuthenticationInBrowser) {
    SettingsHolder* settingsHolder = SettingsHolder::instance();
    settingsHolder->setPendingAuthCodeVerifier(
        QString::fromLatin1(m_pkceCodeVerifier));
    settingsHolder->setPendingAuthTime(QDateTime::currentDateTimeUtc());
    SettingsManager::instance()->sync();
  }
#endif

  m_authenticationListener->start(this, challenge, CODE_CHALLENGE_METHOD,
                                  SettingsHolder::instance()->userEmail());
}

void TaskAuthenticate::authenticatePkceSuccess(const QString& code) {
  logger.debug() << "Authentication completed with code:"
                 << logger.sensitive(code);

  clearPendingCodeVerifier();

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

  // On Android, onResume fires after onNewIntent, which would trigger the
  // Custom Tab "closed" callback and falsely abort a successful authentication.
  disconnect(m_authenticationListener, &AuthenticationListener::abortedByUser,
             this, nullptr);

  authenticatePkceSuccess(query.queryItemValue("code"));
}

void TaskAuthenticate::resumeWithCode(const QString& code,
                                      const QByteArray& codeVerifier) {
  Q_ASSERT(!m_authenticationListener);
  Q_ASSERT(!code.isEmpty() && !codeVerifier.isEmpty());
  m_resumeCode = code;
  m_pkceCodeVerifier = codeVerifier;
}

// static
QByteArray TaskAuthenticate::consumePendingCodeVerifier() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  QByteArray verifier = settingsHolder->pendingAuthCodeVerifier().toLatin1();
  QDateTime time = settingsHolder->pendingAuthTime();
  clearPendingCodeVerifier();

  if (verifier.isEmpty()) {
    return QByteArray();
  }

  qint64 age = time.secsTo(QDateTime::currentDateTimeUtc());
  if (!time.isValid() || age < 0 || age > PENDING_AUTH_MAX_AGE_SECS) {
    logger.warning() << "Ignoring expired pending authentication";
    return QByteArray();
  }

  return verifier;
}

// static
void TaskAuthenticate::clearPendingCodeVerifier() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  settingsHolder->removePendingAuthCodeVerifier();
  settingsHolder->removePendingAuthTime();
}
