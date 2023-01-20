/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "authenticationinappsession.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QtNetwork>  // for qpassworddigestor.h

#include "authenticationinapp.h"
#include "authenticationlistener.h"
#include "feature.h"
#include "glean/generated/metrics.h"
#include "gleandeprecated.h"
#include "hawkauth.h"
#include "hkdf.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "telemetry/gleansample.h"

namespace {
Logger logger("AuthenticationInAppSession");

NetworkRequest* fxaHawkPostRequest(Task* parent, const QString& path,
                                   const QByteArray& sessionToken,
                                   const QJsonObject& obj) {
  QUrl url(QString("%1%2").arg(Constants::fxaApiBaseUrl(), path));
  QByteArray payload = QJsonDocument(obj).toJson(QJsonDocument::Compact);

  HawkAuth hawk = HawkAuth(sessionToken);
  QByteArray hawkHeader =
      hawk.generate(url, "POST", "application/json", payload).toUtf8();

  NetworkRequest* request = new NetworkRequest(parent, 200);
  request->requestInternal().setRawHeader("Authorization", hawkHeader);
  request->post(url, payload);

  return request;
}

}  // anonymous namespace

AuthenticationInAppSession::AuthenticationInAppSession(QObject* parent,
                                                       TypeAuthentication type)
    : QObject(parent), m_typeAuthentication(type) {
  MZ_COUNT_CTOR(AuthenticationInAppSession);
}

AuthenticationInAppSession::~AuthenticationInAppSession() {
  MZ_COUNT_DTOR(AuthenticationInAppSession);
}

void AuthenticationInAppSession::terminate() {
  if (m_sessionToken.isEmpty()) {
    emit terminated();
    return;
  }

  NetworkRequest* request = fxaHawkPostRequest(m_task, "/v1/session/destroy",
                                               m_sessionToken, QJsonObject());

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Failed to destroy the FxA session" << error;
            emit terminated();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray&) {
            logger.error() << "FxA session destroyed";
            emit terminated();
          });
}

void AuthenticationInAppSession::start(Task* task, const QString& codeChallenge,
                                       const QString& codeChallengeMethod,
                                       const QString& emailAddress) {
  logger.debug() << "AuthenticationInAppSession initialized";

  m_task = task;

  AuthenticationInApp* aia = AuthenticationInApp::instance();
  Q_ASSERT(aia);

  aia->registerSession(this);

  QUrl url(AuthenticationListener::createAuthenticationUrl(
      codeChallenge, codeChallengeMethod, emailAddress));

  NetworkRequest* request = new NetworkRequest(task);
  request->get(url);

  connect(request, &NetworkRequest::requestCompleted, this,
          [this, emailAddress = QString(emailAddress)](const QByteArray& data) {
            logger.debug() << "Request completed";

            QJsonDocument json = QJsonDocument::fromJson(data);
            QJsonObject obj = json.object();

            if (!obj["fxa_oauth"].isObject()) {
              logger.error() << "Invalid JSON document. No fxa_oauth";
              emit failed(ErrorHandler::AuthenticationError);
              return;
            }

            QJsonObject fxaObj = obj["fxa_oauth"].toObject();
            if (!fxaObj["params"].isObject()) {
              logger.error() << "Invalid JSON document. No fxa_oauth/params";
              emit failed(ErrorHandler::AuthenticationError);
              return;
            }

            QJsonObject paramsObj = fxaObj["params"].toObject();

#define GETPARAM(x, y, to)                                               \
  if (!paramsObj.contains(y)) {                                          \
    logger.error() << "Invalid JSON document. No fxa_oauth/params" << y; \
    emit failed(ErrorHandler::AuthenticationError);                      \
    return;                                                              \
  }                                                                      \
  m_fxaParams.x = paramsObj[y].to();

            GETPARAM(m_clientId, "client_id", toString);
            GETPARAM(m_deviceId, "device_id", toString);
            GETPARAM(m_state, "state", toString);
            GETPARAM(m_scope, "scope", toString);
            GETPARAM(m_accessType, "access_type", toString);
            GETPARAM(m_flowId, "flow_id", toString);
            GETPARAM(m_flowBeginTime, "flow_begin_time", toString().toDouble);

#undef GETPARAM

            if (emailAddress.isEmpty()) {
              AuthenticationInApp::instance()->requestState(
                  AuthenticationInApp::StateStart, this);
              return;
            }

            checkAccount(emailAddress);
          });

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to fetch the initial request" << error;
            if (error != QNetworkReply::OperationCanceledError) {
              processRequestFailure(error, data);
            }
          });
}

void AuthenticationInAppSession::checkAccount(const QString& emailAddress) {
  logger.debug() << "Authentication starting:"
                 << logger.sensitive(emailAddress);

#ifdef UNIT_TEST
  if (!m_allowUpperCaseEmailAddress) {
#endif
    m_emailAddress = emailAddress.toLower();
#ifdef UNIT_TEST
  } else {
    m_emailAddress = emailAddress;
  }
#endif

  m_emailAddressCaseFix = emailAddress;

  AuthenticationInApp* aia = AuthenticationInApp::instance();
  Q_ASSERT(aia);

  aia->requestEmailAddressChange(this);
  aia->requestState(AuthenticationInApp::StateCheckingAccount, this);

  NetworkRequest* request = new NetworkRequest(m_task, 200);
  request->post(QString("%1/v1/account/status").arg(Constants::fxaApiBaseUrl()),
                QJsonObject{{"email", m_emailAddress}});

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to check the account status" << error;
            processRequestFailure(error, data);
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Account status checked:"
                           << logger.sensitive(data);

            QJsonDocument json = QJsonDocument::fromJson(data);
            QJsonObject obj = json.object();

            accountChecked(obj["exists"].toBool());
          });
}

void AuthenticationInAppSession::accountChecked(bool exists) {
  logger.debug() << "Account checked:" << exists;

  if (exists) {
    AuthenticationInApp::instance()->requestState(
        AuthenticationInApp::StateSignIn, this);
    return;
  }

  if (Feature::get(Feature::Feature_inAppAccountCreate)->isSupported()) {
    AuthenticationInApp::instance()->requestState(
        AuthenticationInApp::StateSignUp, this);
    return;
  }

  AuthenticationInApp::instance()->requestState(
      AuthenticationInApp::StateFallbackInBrowser, this);

  emit fallbackRequired();
}

QByteArray AuthenticationInAppSession::generateAuthPw() const {
  // Process the user's password into an FxA auth token
  QString salt = QString("identity.mozilla.com/picl/v1/quickStretch:%1")
                     .arg(m_emailAddressCaseFix);
  QByteArray pbkdf = QPasswordDigestor::deriveKeyPbkdf2(
      QCryptographicHash::Sha256, m_password.toUtf8(), salt.toUtf8(), 1000, 32);

  HKDF hash(QCryptographicHash::Sha256);
  hash.addData(pbkdf);

  return hash.result(32, "identity.mozilla.com/picl/v1/authPW");
}

void AuthenticationInAppSession::setPassword(const QString& password) {
  m_password = password;
}

#ifdef UNIT_TEST
void AuthenticationInAppSession::enableTotpCreation() {
  logger.debug() << "Enabling totp creation";
  Q_ASSERT(m_extraOp == OpNone);
  m_extraOp = OpTotpCreationNeeded;
}

void AuthenticationInAppSession::allowUpperCaseEmailAddress() {
  logger.debug() << "Forcing an upper email address";
  m_allowUpperCaseEmailAddress = true;
}
#endif

void AuthenticationInAppSession::signIn(const QString& unblockCode) {
  logger.debug() << "Sign in";

  AuthenticationInApp::instance()->requestState(
      AuthenticationInApp::StateSigningIn, this);

  signInInternal(unblockCode);
}

void AuthenticationInAppSession::signInInternal(const QString& unblockCode) {
  QJsonObject obj{
      {"email", m_emailAddressCaseFix},
      {"authPW", QString(generateAuthPw().toHex())},
      {"reason", "signin"},
      {"service", m_fxaParams.m_clientId},
      {"skipErrorCase", true},
      {"verificationMethod", "email-otp"},
      {"metricsContext",
       QJsonObject{{"deviceId", m_fxaParams.m_deviceId},
                   {"flowId", m_fxaParams.m_flowId},
                   {"flowBeginTime", m_fxaParams.m_flowBeginTime}}}};

  if (!m_originalLoginEmailAddress.isEmpty()) {
    obj.insert("originalLoginEmail", m_originalLoginEmailAddress);
  }

  if (!unblockCode.isEmpty()) {
    obj.insert("unblockCode", unblockCode);
  }

  NetworkRequest* request = new NetworkRequest(m_task, 200);
  request->post(QString("%1/v1/account/login").arg(Constants::fxaApiBaseUrl()),
                obj);

  connect(request, &NetworkRequest::requestFailed, this,
          [this, unblockCode](QNetworkReply::NetworkError error,
                              const QByteArray& data) {
            if (error == QNetworkReply::TimeoutError) {
              AuthenticationInApp* aia = AuthenticationInApp::instance();
              aia->requestState(AuthenticationInApp::StateSignIn, this);
              aia->requestErrorPropagation(
                  this, AuthenticationInApp::ErrorConnectionTimeout);
              return;
            }

            QJsonDocument json = QJsonDocument::fromJson(data);
            if (json.isObject()) {
              QJsonObject obj = json.object();

              int errorCode = obj["errno"].toInt();

              // Incorrect email case
              if (errorCode == 120) {
                QString email = obj["email"].toString();
                if (!email.isEmpty()) {
                  logger.error()
                      << "Failed to sign in for email case issues. New email:"
                      << logger.sensitive(email);
                  m_originalLoginEmailAddress = m_emailAddressCaseFix;
                  m_emailAddressCaseFix = email;
                  signInInternal(unblockCode);
                  return;
                }
              }
            }

            logger.error() << "Failed to sign in" << error;
            processRequestFailure(error, data);
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Sign in completed:" << logger.sensitive(data);

            QJsonDocument json = QJsonDocument::fromJson(data);
            QJsonObject obj = json.object();

            signInOrUpCompleted(obj["sessionToken"].toString(),
                                obj["verified"].toBool(),
                                obj["verificationMethod"].toString());
          });
}

void AuthenticationInAppSession::signUp() {
  logger.debug() << "Sign up";

  AuthenticationInApp::instance()->requestState(
      AuthenticationInApp::StateSigningUp, this);

  NetworkRequest* request = new NetworkRequest(m_task, 200);
  request->post(QString("%1/v1/account/create").arg(Constants::fxaApiBaseUrl()),
                QJsonObject{
                    {
                        "email",
                        m_emailAddressCaseFix,
                    },
                    {"authPW", QString(generateAuthPw().toHex())},
                    {
                        "service",
                        m_fxaParams.m_clientId,
                    },
                    {"verificationMethod", "email-otp"},
                    {"metricsContext",
                     QJsonObject{
                         {"deviceId", m_fxaParams.m_deviceId},
                         {"flowId", m_fxaParams.m_flowId},
                         {"flowBeginTime", m_fxaParams.m_flowBeginTime},
                     }},
                });

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to sign up" << error;
            processRequestFailure(error, data);
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Sign up completed:" << logger.sensitive(data);

            QJsonDocument json = QJsonDocument::fromJson(data);
            QJsonObject obj = json.object();

            signInOrUpCompleted(obj["sessionToken"].toString(),
                                obj["verified"].toBool(),
                                obj["verificationMethod"].toString());
          });
}

void AuthenticationInAppSession::unblockCodeNeeded() {
  logger.debug() << "Unblock code needed";
  AuthenticationInApp::instance()->requestState(
      AuthenticationInApp::StateUnblockCodeNeeded, this);
  sendUnblockCodeEmail();
}

void AuthenticationInAppSession::verifyUnblockCode(const QString& unblockCode) {
  logger.debug() << "Sign in (unblock code received)";
  Q_ASSERT(m_sessionToken.isEmpty());

  AuthenticationInApp::instance()->requestState(
      AuthenticationInApp::StateVerifyingUnblockCode, this);

  signInInternal(unblockCode);
}

void AuthenticationInAppSession::sendUnblockCodeEmail() {
  logger.debug() << "Resend unblock code";
  Q_ASSERT(m_sessionToken.isEmpty());

  NetworkRequest* request = new NetworkRequest(m_task, 200);
  request->post(QString("%1/v1/account/login/send_unblock_code")
                    .arg(Constants::fxaApiBaseUrl()),
                QJsonObject{{"email", m_emailAddressCaseFix}});

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to resend the unblock code" << error;
            processRequestFailure(error, data);
          });

  connect(request, &NetworkRequest::requestCompleted,
          [](const QByteArray& data) {
            logger.debug() << "Code resent:" << logger.sensitive(data);
          });
}

void AuthenticationInAppSession::verifySessionEmailCode(const QString& code) {
  logger.debug() << "Sign in (verify session code by email received)";
  Q_ASSERT(!m_sessionToken.isEmpty());

  AuthenticationInApp::instance()->requestState(
      AuthenticationInApp::StateVerifyingSessionEmailCode, this);

  QJsonArray scopes;
  QStringList queryScopes = m_fxaParams.m_scope.split(" ");
  foreach (const QString& s, queryScopes) {
    QString parsedScope;
    if (s.startsWith("http")) {
      parsedScope = QUrl::fromPercentEncoding(s.toUtf8());
    } else {
      parsedScope = s;
    }
    scopes.append(parsedScope);
  }

  NetworkRequest* request =
      fxaHawkPostRequest(m_task, "/v1/session/verify_code", m_sessionToken,
                         QJsonObject{{"code", code},
                                     {"service", m_fxaParams.m_clientId},
                                     {"scopes", scopes}});

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to verify the session code" << error;
            processRequestFailure(error, data);
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Verification completed:"
                           << logger.sensitive(data);
            finalizeSignInOrUp();
          });
}

void AuthenticationInAppSession::resendVerificationSessionCodeEmail() {
  logger.debug() << "Resend verification code";
  Q_ASSERT(!m_sessionToken.isEmpty());

  NetworkRequest* request = fxaHawkPostRequest(
      m_task, "/v1/session/resend_code", m_sessionToken, QJsonObject());

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to resend the session code" << error;
            processRequestFailure(error, data);
          });

  connect(request, &NetworkRequest::requestCompleted,
          [](const QByteArray& data) {
            logger.debug() << "Code resent:" << logger.sensitive(data);
          });
}

void AuthenticationInAppSession::verifySessionTotpCode(const QString& code) {
  logger.debug() << "Sign in (verify session code by totp received)";
  Q_ASSERT(!m_sessionToken.isEmpty());

  AuthenticationInApp::instance()->requestState(
      AuthenticationInApp::StateVerifyingSessionTotpCode, this);

  NetworkRequest* request = fxaHawkPostRequest(
      m_task, "/v1/session/verify/totp", m_sessionToken,
      QJsonObject{{"code", code},
                  {"service", m_fxaParams.m_clientId},
                  {"scopes", QJsonArray{m_fxaParams.m_scope}}});

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to verify the Totp code" << error;
            processRequestFailure(error, data);
          });

  connect(
      request, &NetworkRequest::requestCompleted, this,
      [this](const QByteArray& data) {
        logger.debug() << "Verification completed:" << logger.sensitive(data);

        QJsonDocument json = QJsonDocument::fromJson(data);
        if (json.isNull()) {
          emit failed(ErrorHandler::AuthenticationError);
          return;
        }

        QJsonObject obj = json.object();
        bool success = obj.value("success").toBool();
        if (success) {
          finalizeSignInOrUp();
          return;
        }

        AuthenticationInApp* aia = AuthenticationInApp::instance();
        aia->requestState(
            AuthenticationInApp::StateVerificationSessionByTotpNeeded, this);
        aia->requestErrorPropagation(this,
                                     AuthenticationInApp::ErrorInvalidTotpCode);
      });
}

void AuthenticationInAppSession::signInOrUpCompleted(
    const QString& sessionToken, bool accountVerified,
    const QString& verificationMethod) {
  logger.debug() << "Session generated";

  logger.debug() << "FxA Session Token:" << logger.sensitive(sessionToken);

  // Let's store it to delete it at the DTOR.
  m_sessionToken = QByteArray::fromHex(sessionToken.toUtf8());

  if (!accountVerified) {
    if (verificationMethod == "totp-2fa") {
      AuthenticationInApp::instance()->requestState(
          AuthenticationInApp::StateVerificationSessionByTotpNeeded, this);
      return;
    }

    if (verificationMethod == "email-otp") {
      AuthenticationInApp::instance()->requestState(
          AuthenticationInApp::StateVerificationSessionByEmailNeeded, this);
      return;
    }

    logger.error() << "Unsupported verification method:" << verificationMethod;
    return;
  }

  finalizeSignInOrUp();
}

#ifdef UNIT_TEST
void AuthenticationInAppSession::createTotpCodes() {
  NetworkRequest* request = fxaHawkPostRequest(m_task, "/v1/totp/create",
                                               m_sessionToken, QJsonObject());

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to create totp codes" << error;
            processRequestFailure(error, data);
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Totp code creation completed:"
                           << logger.sensitive(data);

            AuthenticationInApp* aia = AuthenticationInApp::instance();
            aia->requestState(
                AuthenticationInApp::StateVerificationSessionByTotpNeeded,
                this);
            emit aia->unitTestTotpCodeCreated(data);
          });
}
#endif

void AuthenticationInAppSession::startAccountDeletionFlow() {
  QUrl url(QString("%1/v1/account/attached_clients")
               .arg(Constants::fxaApiBaseUrl()));

  HawkAuth hawk = HawkAuth(m_sessionToken);
  QByteArray hawkHeader = hawk.generate(url, "GET", "", "").toUtf8();

  NetworkRequest* request = new NetworkRequest(m_task, 200);
  request->requestInternal().setRawHeader("Authorization", hawkHeader);
  request->get(url);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to fetch attach clients" << error;
            processRequestFailure(error, data);
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Attached clients:" << logger.sensitive(data);

            QJsonDocument json = QJsonDocument::fromJson(data);
            if (json.isNull()) {
              emit failed(ErrorHandler::AuthenticationError);
              return;
            }

            const QJsonArray clientArray = json.array();
            for (const QJsonValue& clientValue : clientArray) {
              if (!clientValue.isObject()) {
                logger.error() << "Attach clients: no client object found";
                emit failed(ErrorHandler::AuthenticationError);
                return;
              }

              QJsonObject clientObj = clientValue.toObject();
              QJsonValue clientName = clientObj["name"];
              if (!clientName.isString()) {
                logger.error() << "Attach clients: no client name found";
                emit failed(ErrorHandler::AuthenticationError);
                return;
              }

              QString clientNameStr = clientName.toString();
              if (clientNameStr.isEmpty() ||
                  m_attachedClients.contains(clientNameStr)) {
                continue;
              }

              m_attachedClients.append(clientNameStr);
            }

            AuthenticationInApp* aia = AuthenticationInApp::instance();
            aia->requestAttachedClientsChange(this);

            aia->requestState(AuthenticationInApp::StateAccountDeletionRequest,
                              this);
          });
}

void AuthenticationInAppSession::deleteAccount() {
  NetworkRequest* request = fxaHawkPostRequest(
      m_task, "/v1/account/destroy", m_sessionToken,
      QJsonObject{{"email", m_emailAddress},
                  {"authPW", QString(generateAuthPw().toHex())}});

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Failed to delete the account" << error;
            emit accountDeleted();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Account deleted" << logger.sensitive(data);
            emit accountDeleted();
          });

  mozilla::glean::sample::delete_account_clicked.record();
  emit GleanDeprecated::instance()->recordGleanEvent(
      GleanSample::deleteAccountClicked);
}

void AuthenticationInAppSession::finalizeSignInOrUp() {
  Q_ASSERT(!m_sessionToken.isEmpty());

#ifdef UNIT_TEST
  if (m_extraOp == OpTotpCreationNeeded) {
    logger.info() << "Totp creation in process";
    // Let's set it to `OpNone` to avoid loops at the next finalizeSignInOrUp()
    // call.
    m_extraOp = OpNone;
    createTotpCodes();
    return;
  }
#endif

  NetworkRequest* request = fxaHawkPostRequest(
      m_task, "/v1/oauth/authorization", m_sessionToken,
      QJsonObject{{"client_id", m_fxaParams.m_clientId},
                  {"state", m_fxaParams.m_state},
                  {"scope", m_fxaParams.m_scope},
                  {"access_type", m_fxaParams.m_accessType}});

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to create oauth code" << error;
            processRequestFailure(error, data);
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Oauth code creation completed:"
                           << logger.sensitive(data);

            QJsonDocument json = QJsonDocument::fromJson(data);
            if (json.isNull()) {
              emit failed(ErrorHandler::AuthenticationError);
              return;
            }

            QJsonObject obj = json.object();
            QJsonValue code = obj.value("code");
            if (!code.isString()) {
              logger.error() << "FxA Authz: code not found";
              emit failed(ErrorHandler::AuthenticationError);
              return;
            }
            QJsonValue state = obj.value("state");
            if (!state.isString()) {
              logger.error() << "FxA Authz: state not found";
              emit failed(ErrorHandler::AuthenticationError);
              return;
            }
            QJsonValue redirect = obj.value("redirect");
            if (!redirect.isString()) {
              logger.error() << "FxA Authz: redirect not found";
              emit failed(ErrorHandler::AuthenticationError);
              return;
            }

            NetworkRequest* request = new NetworkRequest(m_task, 200);
            request->requestInternal().setAttribute(
                QNetworkRequest::RedirectPolicyAttribute,
                QNetworkRequest::NoLessSafeRedirectPolicy);
            request->get(redirect.toString());

            connect(request, &NetworkRequest::requestFailed, this,
                    [this](QNetworkReply::NetworkError error,
                           const QByteArray& data) {
                      QJsonDocument json = QJsonDocument::fromJson(data);
                      if (!json.isObject()) {
                        emit failed(ErrorHandler::toErrorType(error));
                        return;
                      }

                      QJsonObject obj = json.object();
                      QString detail = obj["detail"].toString();
                      if (detail.isEmpty()) {
                        logger.error() << "Invalid JSON: no detail value";
                        emit failed(ErrorHandler::AuthenticationError);
                        return;
                      }

                      logger.error() << "Authentication failed:" << detail;
                      emit failed(ErrorHandler::AuthenticationError);
                    });

            connect(request, &NetworkRequest::requestCompleted, this,
                    [this](const QByteArray& data) {
                      logger.debug()
                          << "Final redirect fetch completed:" << data;

                      QJsonDocument json = QJsonDocument::fromJson(data);
                      if (json.isNull()) {
                        emit failed(ErrorHandler::AuthenticationError);
                        return;
                      }

                      QJsonObject obj = json.object();
                      QJsonValue code = obj.value("code");
                      if (!code.isString()) {
                        logger.error() << "Code not received!";
                        emit failed(ErrorHandler::AuthenticationError);
                        return;
                      }

                      emit completed(code.toString());
                    });
          });
}

void AuthenticationInAppSession::processErrorObject(const QJsonObject& obj) {
  AuthenticationInApp* aia = AuthenticationInApp::instance();
  Q_ASSERT(aia);

  int errorCode = obj["errno"].toInt();

  // See
  // https://github.com/mozilla/fxa/blob/main/packages/fxa-auth-server/docs/api.md#defined-errors
  switch (errorCode) {
    case 101:  // Account already exists
      aia->requestState(AuthenticationInApp::StateStart, this);
      aia->requestErrorPropagation(
          this, AuthenticationInApp::ErrorAccountAlreadyExists);
      break;

    case 102:  // Unknown account
      aia->requestState(AuthenticationInApp::StateStart, this);
      aia->requestErrorPropagation(this,
                                   AuthenticationInApp::ErrorUnknownAccount);
      break;

    case 103:  // Incorrect password
      aia->requestState(AuthenticationInApp::StateSignIn, this);
      aia->requestErrorPropagation(this,
                                   AuthenticationInApp::ErrorIncorrectPassword);
      break;

    case 107: {  // Invalid parameter in request body
      QJsonObject objValidation = obj["validation"].toObject();
      const QJsonArray keyArray = objValidation["keys"].toArray();
      QStringList keys;
      for (const QJsonValue& key : keyArray) {
        if (key.isString()) {
          keys.append(key.toString());
        }
      }

      if (keys.contains("unblockCode")) {
        AuthenticationInApp* aia = AuthenticationInApp::instance();
        aia->requestState(AuthenticationInApp::StateUnblockCodeNeeded, this);
        aia->requestErrorPropagation(
            this, AuthenticationInApp::ErrorInvalidUnblockCode);
        break;
      }

      if (keys.contains("email")) {
        AuthenticationInApp* aia = AuthenticationInApp::instance();
        aia->requestState(AuthenticationInApp::StateStart, this);
        aia->requestErrorPropagation(
            this, AuthenticationInApp::ErrorInvalidEmailAddress);
        break;
      }

      if (keys.contains("code")) {
        AuthenticationInApp* aia = AuthenticationInApp::instance();
        // Code invalid can be received both in totp and in email verification
        // steps. We need to check the current step in order to send the correct
        // message.
        if (aia->state() ==
            AuthenticationInApp::StateVerifyingSessionTotpCode) {
          aia->requestState(
              AuthenticationInApp::StateVerificationSessionByTotpNeeded, this);
          aia->requestErrorPropagation(
              this, AuthenticationInApp::ErrorInvalidTotpCode);
          return;
        }

        aia->requestState(
            AuthenticationInApp::StateVerificationSessionByEmailNeeded, this);
        aia->requestErrorPropagation(
            this, AuthenticationInApp::ErrorInvalidOrExpiredVerificationCode);
        break;
      }

      mozilla::glean::sample::authentication_inapp_error.record(
          mozilla::glean::sample::AuthenticationInappErrorExtra{
              ._errno = "107",
              ._validation = QJsonDocument(objValidation).toJson()});
      emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
          GleanSample::authenticationInappError,
          {{"errno", "107"},
           {"validation", QJsonDocument(objValidation).toJson()}});

      logger.error() << "Unsupported validation parameter";
      break;
    }

    case 114:  // Client has sent too many requests
      if (m_typeAuthentication == TypeDefault) {
        aia->requestState(AuthenticationInApp::StateStart, this);
      } else {
        // For non-default authentication flows, we go back to the password
        // request, because the email request step is implicit.
        aia->requestState(AuthenticationInApp::StateSignIn, this);
      }

      aia->requestErrorPropagation(this,
                                   AuthenticationInApp::ErrorTooManyRequests,
                                   obj["retryAfter"].toInt());
      break;

    case 125: {  // The request was blocked for security reasons
      QString verificationMethod = obj["verificationMethod"].toString();
      if (verificationMethod == "email-captcha") {
        unblockCodeNeeded();
        break;
      }

      mozilla::glean::sample::authentication_inapp_error.record(
          mozilla::glean::sample::AuthenticationInappErrorExtra{
              ._errno = "125", ._verificationmethod = verificationMethod});
      emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
          GleanSample::authenticationInappError,
          {{"errno", "125"}, {"verificationmethod", verificationMethod}});

      logger.error() << "Unsupported verification method:"
                     << verificationMethod;
      break;
    }

    case 127:  // Invalid unblock code
      aia->requestState(AuthenticationInApp::StateUnblockCodeNeeded, this);
      aia->requestErrorPropagation(
          this, AuthenticationInApp::ErrorInvalidUnblockCode);
      break;

    case 142:  // Sign in with this email type is not currently supported
      aia->requestState(AuthenticationInApp::StateStart, this);
      aia->requestErrorPropagation(
          this, AuthenticationInApp::ErrorEmailTypeNotSupported);
      break;

    case 149:  // This email can not currently be used to login
      aia->requestState(AuthenticationInApp::StateStart, this);
      aia->requestErrorPropagation(
          this, AuthenticationInApp::ErrorEmailCanNotBeUsedToLogin);
      break;

    case 151:  // Failed to send email
      aia->requestState(AuthenticationInApp::StateSignIn, this);
      aia->requestErrorPropagation(this,
                                   AuthenticationInApp::ErrorFailedToSendEmail);
      break;

    case 183:  // Invalid or expired verification code
      aia->requestState(
          AuthenticationInApp::StateVerificationSessionByEmailNeeded, this);
      aia->requestErrorPropagation(
          this, AuthenticationInApp::ErrorInvalidOrExpiredVerificationCode);
      break;

    case 201:  // Service unavailable
      aia->requestState(AuthenticationInApp::StateStart, this);
      aia->requestErrorPropagation(this,
                                   AuthenticationInApp::ErrorServerUnavailable);
      break;

    case 100:  // Incorrect Database Patch Level
      [[fallthrough]];
    case 104:  // Unverified account
      [[fallthrough]];
    case 105:  // Invalid verification code
      [[fallthrough]];
    case 106:  // Invalid JSON in request body
      [[fallthrough]];
    case 108:  // Missing parameter in request body
      [[fallthrough]];
    case 109:  // Invalid request signature
      [[fallthrough]];
    case 110:  // Invalid authentication token in request signature
      [[fallthrough]];
    case 111:  // Invalid timestamp in request signature
      [[fallthrough]];
    case 112:  // Missing content-length header
      [[fallthrough]];
    case 113:  // Request body too large
      [[fallthrough]];
    case 115:  // Invalid nonce in request signature
      [[fallthrough]];
    case 116:  // This endpoint is no longer supported
      [[fallthrough]];
    case 120:  // Incorrect email case
      [[fallthrough]];
    case 123:  // Unknown device
      [[fallthrough]];
    case 124:  // Session already registered by another device
      [[fallthrough]];
    case 126:  // Account must be reset
      [[fallthrough]];
    case 131:  // Invalid message id
      [[fallthrough]];
    case 132:  // Message rejected
      [[fallthrough]];
    case 133:  // Email account sent complaint
      [[fallthrough]];
    case 134:  // Email account hard bounced
      [[fallthrough]];
    case 135:  // Email account soft bounced
      [[fallthrough]];
    case 136:  // Email already exists
      [[fallthrough]];
    case 137:  // Can not delete primary email
      [[fallthrough]];
    case 138:  // Unverified session
      [[fallthrough]];
    case 139:  // Can not add secondary email that is same as your primary
      [[fallthrough]];
    case 140:  // Email already exists
      [[fallthrough]];
    case 141:  // Email already exists
      [[fallthrough]];
    case 143:  // Unknown email
      [[fallthrough]];
    case 144:  // Email already exists
      [[fallthrough]];
    case 145:  // Reset password with this email type is not currently supported
      [[fallthrough]];
    case 146:  // Invalid signin code
      [[fallthrough]];
    case 147:  // Can not change primary email to an unverified email
      [[fallthrough]];
    case 148:  // Can not change primary email to an email that does not belong
               // to this account
      [[fallthrough]];
    case 150:  // Can not resend email code to an email that does not belong to
               // this account
      [[fallthrough]];
    case 152:  // Invalid token verification code
      [[fallthrough]];
    case 153:  // Expired token verification code
      [[fallthrough]];
    case 154:  // TOTP token already exists for this account.
      [[fallthrough]];
    case 155:  // TOTP token not found.
      [[fallthrough]];
    case 156:  // Recovery code not found.
      [[fallthrough]];
    case 157:  // Unavailable device command.
      [[fallthrough]];
    case 158:  // Recovery key not found.
      [[fallthrough]];
    case 159:  // Recovery key is not valid.
      [[fallthrough]];
    case 160:  // This request requires two step authentication enabled on
               // your account.
      [[fallthrough]];
    case 161:  // Recovery key already exists.
      [[fallthrough]];
    case 162:  // Unknown client_id
      [[fallthrough]];
    case 164:  // Stale auth timestamp
      [[fallthrough]];
    case 165:  // Redis WATCH detected a conflicting update
      [[fallthrough]];
    case 166:  // Not a public client
      [[fallthrough]];
    case 167:  // Incorrect redirect URI
      [[fallthrough]];
    case 168:  // Invalid response_type
      [[fallthrough]];
    case 169:  // Requested scopes are not allowed
      [[fallthrough]];
    case 170:  // Public clients require PKCE OAuth parameters
      [[fallthrough]];
    case 171:  // Required Authentication Context Reference values could not be
               // satisfied
      [[fallthrough]];
    case 176:  // Unknown subscription
      [[fallthrough]];
    case 177:  // Unknown subscription plan
      [[fallthrough]];
    case 178:  // Subscription payment token rejected
      [[fallthrough]];
    case 202:  // Feature not enabled
      [[fallthrough]];
    case 203:  // A backend service request failed.
      [[fallthrough]];
    case 998:  // An internal validation check failed.
      [[fallthrough]];
    default:
      mozilla::glean::sample::authentication_inapp_error.record(
          mozilla::glean::sample::AuthenticationInappErrorExtra{
              ._errno = QString::number(errorCode),
              ._error = obj["error"].toString(),
              ._message = obj["message"].toString()});
      emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
          GleanSample::authenticationInappError,
          {{"errno", QString::number(errorCode)},
           {"error", obj["error"].toString()},
           {"message", obj["message"].toString()}});
      logger.error() << "Unsupported error code:" << errorCode;
      break;
  }
}

void AuthenticationInAppSession::processRequestFailure(
    QNetworkReply::NetworkError error, const QByteArray& data) {
  QJsonDocument json = QJsonDocument::fromJson(data);
  if (json.isObject()) {
    processErrorObject(json.object());
    return;
  }

  emit failed(ErrorHandler::toErrorType(error));
}

void AuthenticationInAppSession::reset() {
  m_sessionToken.clear();

  m_emailAddress.clear();
  m_emailAddressCaseFix.clear();

  AuthenticationInApp* aia = AuthenticationInApp::instance();
  Q_ASSERT(aia);

  aia->requestEmailAddressChange(this);
}
