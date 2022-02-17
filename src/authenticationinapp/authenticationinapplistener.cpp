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

  AuthenticationInApp* aip = AuthenticationInApp::instance();
  Q_ASSERT(aip);

  aip->registerListener(this);

  QUrl url(createAuthenticationUrl(MozillaVPN::AuthenticationInApp,
                                   codeChallenge, codeChallengeMethod,
                                   emailAddress));

  NetworkRequest* request =
      NetworkRequest::createForGetUrl(task, url.toString());

  connect(request, &NetworkRequest::requestRedirected, this,
          [this](NetworkRequest* request, const QUrl& url) {
            logger.debug() << "Redirect received";
            m_urlQuery = QUrlQuery(url.query());

            if (!m_urlQuery.hasQueryItem("client_id")) {
              logger.error() << "No `client_id` token. Unable to proceed";
              emit failed(ErrorHandler::AuthenticationError);
              return;
            }

            if (!m_urlQuery.hasQueryItem("state")) {
              logger.error() << "No `state` token. Unable to proceed";
              emit failed(ErrorHandler::AuthenticationError);
              return;
            }

            AuthenticationInApp::instance()->requestState(
                AuthenticationInApp::StateStart, this);
            request->abort();
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

  m_emailAddress = emailAddress;

  AuthenticationInApp* aip = AuthenticationInApp::instance();
  Q_ASSERT(aip);

  aip->requestEmailAddressChange(this);
  aip->requestState(AuthenticationInApp::StateCheckingAccount, this);

  NetworkRequest* request =
      NetworkRequest::createForFxaAccountStatus(m_task, m_emailAddress);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to check the account status" << error;
            processRequestFailure(error, data);
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Account status checked" << data;

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

QByteArray AuthenticationInAppListener::generateAuthPw(
    const QString& password) const {
  // Process the user's password into an FxA auth token
  QString salt = QString("identity.mozilla.com/picl/v1/quickStretch:%1")
                     .arg(m_emailAddress);
  QByteArray pbkdf = QPasswordDigestor::deriveKeyPbkdf2(
      QCryptographicHash::Sha256, password.toUtf8(), salt.toUtf8(), 1000, 32);

  HKDF hash(QCryptographicHash::Sha256);
  hash.addData(pbkdf);

  return hash.result(32, "identity.mozilla.com/picl/v1/authPW");
}

void AuthenticationInAppListener::setPassword(const QString& password) {
  m_authPw = generateAuthPw(password);
}

#ifdef UNIT_TEST
void AuthenticationInAppListener::enableTotpCreation() {
  logger.debug() << "Enabling totp creation";
  m_totpCreationNeeded = true;
}
#endif

void AuthenticationInAppListener::signIn(const QString& unblockCode) {
  logger.debug() << "Sign in";

  AuthenticationInApp::instance()->requestState(
      AuthenticationInApp::StateSigningIn, this);

  NetworkRequest* request = NetworkRequest::createForFxaLogin(
      m_task, m_emailAddress, m_authPw, unblockCode, m_urlQuery);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to sign in" << error;
            processRequestFailure(error, data);
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Sign in completed" << data;

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
      m_task, m_emailAddress, m_authPw, m_urlQuery);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to sign up" << error;
            processRequestFailure(error, data);
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Sign up completed" << data;

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

void AuthenticationInAppListener::setUnblockCodeAndContinue(
    const QString& unblockCode) {
  logger.debug() << "Sign in (unblock code received)";
  Q_ASSERT(m_sessionToken.isEmpty());
  signIn(unblockCode);
}

void AuthenticationInAppListener::sendUnblockCodeEmail() {
  logger.debug() << "Resend unblock code";
  Q_ASSERT(m_sessionToken.isEmpty());

  NetworkRequest* request =
      NetworkRequest::createForFxaSendUnblockCode(m_task, m_emailAddress);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to resend the unblock code" << error;
            processRequestFailure(error, data);
          });

  connect(
      request, &NetworkRequest::requestCompleted,
      [](const QByteArray& data) { logger.debug() << "Code resent" << data; });
}

void AuthenticationInAppListener::verifySessionEmailCode(const QString& code) {
  logger.debug() << "Sign in (verify session code by email received)";
  Q_ASSERT(!m_sessionToken.isEmpty());

  AuthenticationInApp::instance()->requestState(
      AuthenticationInApp::StateVerifyingSessionEmailCode, this);

  NetworkRequest* request =
      NetworkRequest::createForFxaSessionVerifyByEmailCode(
          m_task, m_sessionToken, code, m_urlQuery);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to verify the session code" << error;
            processRequestFailure(error, data);
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Verification completed" << data;
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

  connect(
      request, &NetworkRequest::requestCompleted,
      [](const QByteArray& data) { logger.debug() << "Code resent" << data; });
}

void AuthenticationInAppListener::verifySessionTotpCode(const QString& code) {
  logger.debug() << "Sign in (verify session code by totp received)";
  Q_ASSERT(!m_sessionToken.isEmpty());

  AuthenticationInApp::instance()->requestState(
      AuthenticationInApp::StateVerifyingSessionTotpCode, this);

  NetworkRequest* request = NetworkRequest::createForFxaSessionVerifyByTotpCode(
      m_task, m_sessionToken, code, m_urlQuery);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to verify the session code" << error;
            processRequestFailure(error, data);
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Verification completed" << data;

            QJsonDocument json = QJsonDocument::fromJson(data);
            if (json.isNull()) {
              MozillaVPN::instance()->errorHandle(
                  ErrorHandler::AuthenticationError);
              return;
            }

            QJsonObject obj = json.object();
            bool success = obj.value("success").toBool();
            if (success) {
              finalizeSignInOrUp();
              return;
            }

            AuthenticationInApp* aip = AuthenticationInApp::instance();
            aip->requestState(
                AuthenticationInApp::StateVerificationSessionByTotpNeeded,
                this);
            aip->requestErrorPropagation(
                AuthenticationInApp::ErrorInvalidTotpCode, this);
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
            logger.debug() << "Totp code creation completed" << data;

            AuthenticationInApp* aip = AuthenticationInApp::instance();
            aip->requestState(
                AuthenticationInApp::StateVerificationSessionByTotpNeeded,
                this);
            emit aip->unitTestTotpCodeCreated(data);
          });
}
#endif

void AuthenticationInAppListener::finalizeSignInOrUp() {
  Q_ASSERT(!m_sessionToken.isEmpty());

#ifdef UNIT_TEST
  if (m_totpCreationNeeded) {
    logger.info() << "Totp creation in process";
    m_totpCreationNeeded = false;
    // Let's set it to false to avoid loops at the next finalizeSignInOrUp()
    // call.
    createTotpCodes();
    return;
  }
#endif

  NetworkRequest* request =
      NetworkRequest::createForFxaAuthz(m_task, m_sessionToken, m_urlQuery);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray& data) {
            logger.error() << "Failed to create oauth code" << error;
            processRequestFailure(error, data);
          });

  connect(
      request, &NetworkRequest::requestCompleted, this,
      [this](const QByteArray& data) {
        logger.debug() << "Oauth code creation completed" << data;

        QJsonDocument json = QJsonDocument::fromJson(data);
        if (json.isNull()) {
          MozillaVPN::instance()->errorHandle(
              ErrorHandler::AuthenticationError);
          return;
        }

        QJsonObject obj = json.object();
        QJsonValue code = obj.value("code");
        if (!code.isString()) {
          logger.error() << "FxA Authz: code not found";
          MozillaVPN::instance()->errorHandle(
              ErrorHandler::AuthenticationError);
          return;
        }
        QJsonValue state = obj.value("state");
        if (!state.isString()) {
          logger.error() << "FxA Authz: state not found";
          MozillaVPN::instance()->errorHandle(
              ErrorHandler::AuthenticationError);
          return;
        }
        QJsonValue redirect = obj.value("redirect");
        if (!redirect.isString()) {
          logger.error() << "FxA Authz: redirect not found";
          MozillaVPN::instance()->errorHandle(
              ErrorHandler::AuthenticationError);
          return;
        }

        NetworkRequest* request =
            NetworkRequest::createForGetUrl(m_task, redirect.toString(), 200);

        connect(
            request, &NetworkRequest::requestFailed, this,
            [this](QNetworkReply::NetworkError error, const QByteArray& data) {
              logger.error()
                  << "Failed to fetch the final redirect data" << error;
              processRequestFailure(error, data);
            });

        connect(request, &NetworkRequest::requestHeaderReceived, this,
                [this](NetworkRequest* request) {
#ifdef UNIT_TEST
                  AuthenticationInApp* aip = AuthenticationInApp::instance();
                  emit aip->unitTestFinalUrl(request->url());
#endif
                  // On a 200 response, we receive the OAuth code from the
                  // query string
                  QString code =
                      QUrlQuery(request->url()).queryItemValue("code");
                  if (code.isEmpty()) {
                    logger.error() << "Code not received!";
                    MozillaVPN::instance()->errorHandle(
                        ErrorHandler::AuthenticationError);
                    emit failed(ErrorHandler::AuthenticationError);
                    return;
                  }

                  emit completed(code);
                });
      });
}

void AuthenticationInAppListener::processErrorCode(int errorCode) {
  AuthenticationInApp* aip = AuthenticationInApp::instance();
  Q_ASSERT(aip);

  // See
  // https://github.com/mozilla/fxa/blob/main/packages/fxa-auth-server/docs/api.md#defined-errors
  switch (errorCode) {
    case 101:  // Account already exists
      aip->requestState(AuthenticationInApp::StateStart, this);
      aip->requestErrorPropagation(
          AuthenticationInApp::ErrorAccountAlreadyExists, this);
      break;

    case 102:  // Unknown account
      aip->requestState(AuthenticationInApp::StateStart, this);
      aip->requestErrorPropagation(AuthenticationInApp::ErrorUnknownAccount,
                                   this);
      break;

    case 103:  // Incorrect password
      aip->requestState(AuthenticationInApp::StateSignIn, this);
      aip->requestErrorPropagation(AuthenticationInApp::ErrorIncorrectPassword,
                                   this);
      break;

    case 114:  // Client has sent too many requests
      aip->requestState(AuthenticationInApp::StateStart, this);
      aip->requestErrorPropagation(AuthenticationInApp::ErrorTooManyRequests,
                                   this);
      break;

    case 125:  // The request was blocked for security reasons
      Q_ASSERT(false);
      break;

    case 127:  // Invalid unblock code
      aip->requestState(AuthenticationInApp::StateStart, this);
      aip->requestErrorPropagation(AuthenticationInApp::ErrorInvalidEmailCode,
                                   this);
      break;

    case 142:  // Sign in with this email type is not currently supported
      aip->requestState(AuthenticationInApp::StateStart, this);
      aip->requestErrorPropagation(
          AuthenticationInApp::ErrorEmailTypeNotSupported, this);
      break;

    case 144:  // Email already exists
      aip->requestState(AuthenticationInApp::StateStart, this);
      aip->requestErrorPropagation(AuthenticationInApp::ErrorEmailAlreadyExists,
                                   this);
      break;

    case 149:  // This email can not currently be used to login
      aip->requestState(AuthenticationInApp::StateStart, this);
      aip->requestErrorPropagation(
          AuthenticationInApp::ErrorEmailCanNotBeUsedToLogin, this);
      break;

    case 151:  // Failed to send email
      aip->requestState(AuthenticationInApp::StateSignIn, this);
      aip->requestErrorPropagation(AuthenticationInApp::ErrorFailedToSendEmail,
                                   this);
      break;

    case 201:  // Service unavailable
      aip->requestState(AuthenticationInApp::StateStart, this);
      aip->requestErrorPropagation(AuthenticationInApp::ErrorServerUnavailable,
                                   this);
      break;

    case 100:  // Incorrect Database Patch Level
      [[fallthrough]];
    case 104:  // Unverified account
      [[fallthrough]];
    case 105:  // Invalid verification code
      [[fallthrough]];
    case 106:  // Invalid JSON in request body
      [[fallthrough]];
    case 107:  // Invalid parameter in request body
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
      logger.error() << "Unsupported error code:" << errorCode;
      break;
  }
}

void AuthenticationInAppListener::processRequestFailure(
    QNetworkReply::NetworkError error, const QByteArray& data) {
  QJsonDocument json = QJsonDocument::fromJson(data);
  if (json.isObject()) {
    QJsonObject obj = json.object();

    int errorCode = obj["errno"].toInt();

    // The request was blocked for security reasons
    if (errorCode == 125) {
      QString verificationMethod = obj["verificationMethod"].toString();
      if (verificationMethod == "email-captcha") {
        unblockCodeNeeded();
        return;
      }

      logger.error() << "Unsupported verification method:"
                     << verificationMethod;
      return;
    }

    processErrorCode(errorCode);
    return;
  }

  MozillaVPN::instance()->errorHandle(ErrorHandler::toErrorType(error));
  emit failed(ErrorHandler::toErrorType(error));
}
