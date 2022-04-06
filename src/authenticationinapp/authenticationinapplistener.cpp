/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "authenticationinapplistener.h"
#include "authenticationinapp.h"
#include "featurelist.h"
#include "features/featureinappaccountcreate.h"
#include "leakdetector.h"
#include "logger.h"
#include "hkdf.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include "../../glean/telemetry/gleansample.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QtNetwork>  // for qpassworddigestor.h

namespace {
Logger logger(LOG_MAIN, "AuthenticationInAppListener");
}  // anonymous namespace

AuthenticationInAppListener::AuthenticationInAppListener(QObject* parent)
    : AuthenticationListener(parent) {
  MVPN_COUNT_CTOR(AuthenticationInAppListener);
}

AuthenticationInAppListener::~AuthenticationInAppListener() {
  MVPN_COUNT_DTOR(AuthenticationInAppListener);
}

void AuthenticationInAppListener::aboutToFinish() {
  if (m_sessionToken.isEmpty()) {
    emit readyToFinish();
    return;
  }

#ifdef UNIT_TEST
  if (m_extraOp == OpAccountDeletionNeeded) {
    logger.info() << "Deleting account in progress";
    deleteAccount();
    return;
  }
#endif

  NetworkRequest* request =
      NetworkRequest::createForFxaSessionDestroy(m_task, m_sessionToken);
  Q_ASSERT(request);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Failed to destroy the FxA session" << error;
            emit readyToFinish();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray&) {
            logger.error() << "FxA session destroyed";
            emit readyToFinish();
          });
}

void AuthenticationInAppListener::start(Task* task,
                                        const QString& codeChallenge,
                                        const QString& codeChallengeMethod,
                                        const QString& emailAddress) {
  logger.debug() << "AuthenticationInAppListener initialized";

  m_task = task;
  m_codeChallenge = codeChallenge;
  m_codeChallengeMethod = codeChallengeMethod;

  AuthenticationInApp* aia = AuthenticationInApp::instance();
  Q_ASSERT(aia);

  aia->registerListener(this);

  QUrl url(createAuthenticationUrl(codeChallenge, codeChallengeMethod,
                                   emailAddress));

  NetworkRequest* request =
      NetworkRequest::createForGetUrl(task, url.toString());

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
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

            AuthenticationInApp::instance()->requestState(
                AuthenticationInApp::StateStart, this);
          });

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to fetch the initial request" << error;
            if (error != QNetworkReply::OperationCanceledError) {
              processRequestFailure(error, data);
            }
          });
}

void AuthenticationInAppListener::checkAccount(const QString& emailAddress) {
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

  NetworkRequest* request =
      NetworkRequest::createForFxaAccountStatus(m_task, m_emailAddress);

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

void AuthenticationInAppListener::accountChecked(bool exists) {
  logger.debug() << "Account checked:" << exists;

  if (exists) {
    AuthenticationInApp::instance()->requestState(
        AuthenticationInApp::StateSignIn, this);
    return;
  }

  if (FeatureInAppAccountCreate::instance()->isSupported()) {
    AuthenticationInApp::instance()->requestState(
        AuthenticationInApp::StateSignUp, this);
    return;
  }

  AuthenticationInApp::instance()->requestState(
      AuthenticationInApp::StateFallbackInBrowser, this);

  AuthenticationListener* fallbackListener =
      create(this, MozillaVPN::AuthenticationInBrowser);
  fallbackListener->start(m_task, m_codeChallenge, m_codeChallengeMethod,
                          m_emailAddress);

  connect(fallbackListener, &AuthenticationListener::completed, this,
          &AuthenticationListener::completed);
  connect(fallbackListener, &AuthenticationListener::failed, this,
          &AuthenticationListener::failed);
  connect(fallbackListener, &AuthenticationListener::abortedByUser, this,
          &AuthenticationListener::abortedByUser);
}

QByteArray AuthenticationInAppListener::generateAuthPw() const {
  // Process the user's password into an FxA auth token
  QString salt = QString("identity.mozilla.com/picl/v1/quickStretch:%1")
                     .arg(m_emailAddressCaseFix);
  QByteArray pbkdf = QPasswordDigestor::deriveKeyPbkdf2(
      QCryptographicHash::Sha256, m_password.toUtf8(), salt.toUtf8(), 1000, 32);

  HKDF hash(QCryptographicHash::Sha256);
  hash.addData(pbkdf);

  return hash.result(32, "identity.mozilla.com/picl/v1/authPW");
}

void AuthenticationInAppListener::setPassword(const QString& password) {
  m_password = password;
}

#ifdef UNIT_TEST
void AuthenticationInAppListener::enableTotpCreation() {
  logger.debug() << "Enabling totp creation";
  Q_ASSERT(m_extraOp == OpNone);
  m_extraOp = OpTotpCreationNeeded;
}

void AuthenticationInAppListener::enableAccountDeletion() {
  logger.debug() << "Delete account request";
  Q_ASSERT(m_extraOp == OpNone);
  m_extraOp = OpAccountDeletionNeeded;
}

void AuthenticationInAppListener::allowUpperCaseEmailAddress() {
  logger.debug() << "Forcing an upper email address";
  m_allowUpperCaseEmailAddress = true;
}
#endif

void AuthenticationInAppListener::signIn(const QString& unblockCode) {
  logger.debug() << "Sign in";

  AuthenticationInApp::instance()->requestState(
      AuthenticationInApp::StateSigningIn, this);

  signInInternal(unblockCode);
}

void AuthenticationInAppListener::signInInternal(const QString& unblockCode) {
  NetworkRequest* request = NetworkRequest::createForFxaLogin(
      m_task, m_emailAddressCaseFix, generateAuthPw(), unblockCode,
      m_fxaParams.m_clientId, m_fxaParams.m_deviceId, m_fxaParams.m_flowId,
      m_fxaParams.m_flowBeginTime);

  connect(request, &NetworkRequest::requestFailed, this,
          [this, unblockCode](QNetworkReply::NetworkError error,
                              const QByteArray& data) {
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

void AuthenticationInAppListener::signUp() {
  logger.debug() << "Sign up";

  AuthenticationInApp::instance()->requestState(
      AuthenticationInApp::StateSigningUp, this);

  NetworkRequest* request = NetworkRequest::createForFxaAccountCreation(
      m_task, m_emailAddressCaseFix, generateAuthPw(), m_fxaParams.m_clientId,
      m_fxaParams.m_deviceId, m_fxaParams.m_flowId,
      m_fxaParams.m_flowBeginTime);

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

void AuthenticationInAppListener::unblockCodeNeeded() {
  logger.debug() << "Unblock code needed";
  AuthenticationInApp::instance()->requestState(
      AuthenticationInApp::StateUnblockCodeNeeded, this);
  sendUnblockCodeEmail();
}

void AuthenticationInAppListener::verifyUnblockCode(
    const QString& unblockCode) {
  logger.debug() << "Sign in (unblock code received)";
  Q_ASSERT(m_sessionToken.isEmpty());

  AuthenticationInApp::instance()->requestState(
      AuthenticationInApp::StateVerifyingUnblockCode, this);

  signInInternal(unblockCode);
}

void AuthenticationInAppListener::sendUnblockCodeEmail() {
  logger.debug() << "Resend unblock code";
  Q_ASSERT(m_sessionToken.isEmpty());

  NetworkRequest* request = NetworkRequest::createForFxaSendUnblockCode(
      m_task, m_emailAddressCaseFix);

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

void AuthenticationInAppListener::verifySessionEmailCode(const QString& code) {
  logger.debug() << "Sign in (verify session code by email received)";
  Q_ASSERT(!m_sessionToken.isEmpty());

  AuthenticationInApp::instance()->requestState(
      AuthenticationInApp::StateVerifyingSessionEmailCode, this);

  NetworkRequest* request =
      NetworkRequest::createForFxaSessionVerifyByEmailCode(
          m_task, m_sessionToken, code, m_fxaParams.m_clientId,
          m_fxaParams.m_scope);

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

void AuthenticationInAppListener::resendVerificationSessionCodeEmail() {
  logger.debug() << "Resend verification code";
  Q_ASSERT(!m_sessionToken.isEmpty());

  NetworkRequest* request =
      NetworkRequest::createForFxaSessionResendCode(m_task, m_sessionToken);

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

void AuthenticationInAppListener::verifySessionTotpCode(const QString& code) {
  logger.debug() << "Sign in (verify session code by totp received)";
  Q_ASSERT(!m_sessionToken.isEmpty());

  AuthenticationInApp::instance()->requestState(
      AuthenticationInApp::StateVerifyingSessionTotpCode, this);

  NetworkRequest* request = NetworkRequest::createForFxaSessionVerifyByTotpCode(
      m_task, m_sessionToken, code, m_fxaParams.m_clientId,
      m_fxaParams.m_scope);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to verify the session code" << error;
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

void AuthenticationInAppListener::signInOrUpCompleted(
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
void AuthenticationInAppListener::createTotpCodes() {
  NetworkRequest* request =
      NetworkRequest::createForFxaTotpCreation(m_task, m_sessionToken);

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

void AuthenticationInAppListener::deleteAccount() {
  NetworkRequest* request = NetworkRequest::createForFxaAccountDeletion(
      m_task, m_sessionToken, m_emailAddress, generateAuthPw());

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Failed to delete the account" << error;
            emit readyToFinish();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Account deleted" << logger.sensitive(data);

            AuthenticationInApp* aia = AuthenticationInApp::instance();
            aia->requestState(AuthenticationInApp::StateStart, this);
            emit aia->unitTestAccountDeleted();

            emit readyToFinish();
          });
}
#endif

void AuthenticationInAppListener::finalizeSignInOrUp() {
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

  NetworkRequest* request = NetworkRequest::createForFxaAuthz(
      m_task, m_sessionToken, m_fxaParams.m_clientId, m_fxaParams.m_state,
      m_fxaParams.m_scope, m_fxaParams.m_accessType);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to create oauth code" << error;
            processRequestFailure(error, data);
          });

  connect(
      request, &NetworkRequest::requestCompleted, this,
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

        NetworkRequest* request =
            NetworkRequest::createForGetUrl(m_task, redirect.toString(), 200);

        connect(
            request, &NetworkRequest::requestFailed, this,
            [this](QNetworkReply::NetworkError error, const QByteArray& data) {
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

#ifdef UNIT_TEST
              AuthenticationInApp* aia = AuthenticationInApp::instance();
              emit aia->unitTestAuthFailedWithDetail(detail);
#endif

              logger.error() << "Authentication failed:" << detail;
              emit failed(ErrorHandler::AuthenticationError);
            });

        connect(request, &NetworkRequest::requestCompleted, this,
                [this](const QByteArray& data) {
                  logger.debug() << "Final redirect fetch completed:" << data;

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

void AuthenticationInAppListener::processErrorObject(const QJsonObject& obj) {
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
      QStringList keys;
      for (QJsonValue key : objValidation["keys"].toArray()) {
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
        aia->requestState(
            AuthenticationInApp::StateVerificationSessionByEmailNeeded, this);
        aia->requestErrorPropagation(
            this, AuthenticationInApp::ErrorInvalidOrExpiredVerificationCode);
        break;
      }

      emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
          GleanSample::authenticationInappError,
          {{"errno", "107"},
           {"validation", QJsonDocument(objValidation).toJson()}});

      logger.error() << "Unsupported validation parameter";
      break;
    }

    case 114:  // Client has sent too many requests
      aia->requestState(AuthenticationInApp::StateStart, this);
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

      emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
          GleanSample::authenticationInappError,
          {{"errno", "125"}, {"verificationMethod", verificationMethod}});

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

    case 144:  // Email already exists
      aia->requestState(AuthenticationInApp::StateStart, this);
      aia->requestErrorPropagation(
          this, AuthenticationInApp::ErrorEmailAlreadyExists);
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
      emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
          GleanSample::authenticationInappError,
          {{"errno", QString::number(errorCode)},
           {"error", obj["error"].toString()},
           {"message", obj["message"].toString()}});
      logger.error() << "Unsupported error code:" << errorCode;
      break;
  }
}

void AuthenticationInAppListener::processRequestFailure(
    QNetworkReply::NetworkError error, const QByteArray& data) {
  QJsonDocument json = QJsonDocument::fromJson(data);
  if (json.isObject()) {
    processErrorObject(json.object());
    return;
  }

  emit failed(ErrorHandler::toErrorType(error));
}

void AuthenticationInAppListener::reset() {
  m_sessionToken.clear();

  m_emailAddress.clear();
  m_emailAddressCaseFix.clear();

  AuthenticationInApp* aia = AuthenticationInApp::instance();
  Q_ASSERT(aia);

  aia->requestEmailAddressChange(this);
}
