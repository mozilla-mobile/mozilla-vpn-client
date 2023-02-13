/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskgetsubscriptiondetails.h"

#include <QJsonObject>

#include "appconstants.h"
#include "authenticationinapp/authenticationinapp.h"
#include "authenticationinapp/authenticationinappsession.h"
#include "authenticationlistener.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/subscriptiondata.h"
#include "models/user.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskGetSubscriptionDetails");
}  // anonymous namespace

TaskGetSubscriptionDetails::TaskGetSubscriptionDetails(
    AuthenticationPolicy authenticationPolicy,
    ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy)
    : Task("TaskGetSubscriptionDetails"),
      m_authenticationPolicy(authenticationPolicy),
      m_errorPropagationPolicy(errorPropagationPolicy) {
  MZ_COUNT_CTOR(TaskGetSubscriptionDetails);
}

TaskGetSubscriptionDetails::~TaskGetSubscriptionDetails() {
  MZ_COUNT_DTOR(TaskGetSubscriptionDetails);
}

void TaskGetSubscriptionDetails::run() {
  logger.debug() << "run";

  // If this Task is created with `ForceAuthenticationFlow` policy,
  // we are forcing the user to authenticate.
  if (m_authenticationPolicy == ForceAuthenticationFlow) {
    logger.error() << "Force authentication";
    initAuthentication();
    return;
  }

  runInternal();
}

void TaskGetSubscriptionDetails::runInternal() {
  NetworkRequest* request = new NetworkRequest(this, 200);
  request->auth(MozillaVPN::authorizationHeader());
  request->get(AppConstants::apiUrl(AppConstants::SubscriptionDetails));

  connect(
      request, &NetworkRequest::requestFailed, this,
      [this](QNetworkReply::NetworkError error, const QByteArray&) {
        logger.error() << "Get subscription details failed" << error;

        if (error != QNetworkReply::AuthenticationRequiredError) {
          REPORTNETWORKERROR(error, m_errorPropagationPolicy, name());
        } else {
          switch (m_authenticationPolicy) {
            case RunAuthenticationFlowIfNeeded:
              if (!m_authenticationInAppSession) {
                logger.error() << "Needs authentication";
                initAuthentication();
                return;
              }

              logger.error() << "Network request failed after authentication";
              break;

            case ForceAuthenticationFlow:
              logger.error() << "We did the auth before, but somehow it "
                                "was not enough";
              Q_ASSERT(m_authenticationInAppSession);
              break;

            case NoAuthenticationFlow:
              logger.error() << "Needs authentication, but excluded by policy";
              Q_ASSERT(!m_authenticationInAppSession);
              break;
          }
        }

        maybeComplete(false);
      });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Get subscription details completed"
                           << logger.sensitive(data);

            MozillaVPN* vpn = MozillaVPN::instance();
            Q_ASSERT(vpn);

            if (!vpn->subscriptionData()->fromJson(data)) {
              logger.error() << "Failed to parse the Subscription JSON data";
              maybeComplete(false);
              return;
            }

            vpn->subscriptionData()->writeSettings();
            maybeComplete(true);
          });
}

void TaskGetSubscriptionDetails::maybeComplete(bool status) {
  if (!status) {
    MozillaVPN::instance()->subscriptionData()->reset();
  }

  emit operationCompleted(status);

  if (m_authenticationInAppSession) {
    m_authenticationInAppSession->terminate();
    return;
  }

  emit completed();
}

void TaskGetSubscriptionDetails::initAuthentication() {
  logger.debug() << "Init authentication";
  Q_ASSERT(!m_authenticationInAppSession);

  emit needsAuthentication();

  QByteArray pkceCodeVerifier;
  QByteArray pkceCodeChallenge;
  AuthenticationListener::generatePkceCodes(pkceCodeVerifier,
                                            pkceCodeChallenge);
  Q_ASSERT(!pkceCodeVerifier.isEmpty() && !pkceCodeChallenge.isEmpty());

  m_authenticationInAppSession = new AuthenticationInAppSession(
      this, AuthenticationInAppSession::TypeSubscriptionManagement);

  connect(m_authenticationInAppSession, &AuthenticationInAppSession::terminated,
          this, &Task::completed);

  connect(m_authenticationInAppSession, &AuthenticationInAppSession::completed,
          this, [this, pkceCodeVerifier](const QString& pkceCodeSuccess) {
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
                  maybeComplete(false);
                });

            connect(request, &NetworkRequest::requestCompleted, this,
                    [this](const QByteArray&) {
                      logger.debug() << "Authentication completed. Attempt "
                                        "fetching subscription details again.";
                      runInternal();
                    });
          });

  connect(m_authenticationInAppSession, &AuthenticationInAppSession::failed,
          this, [this](const ErrorHandler::ErrorType error) {
            REPORTERROR(error, name());
            maybeComplete(false);
          });

  connect(AuthenticationInApp::instance(), &AuthenticationInApp::stateChanged,
          this, [this] {
            switch (AuthenticationInApp::instance()->state()) {
              case AuthenticationInApp::StateSignUp:
                [[fallthrough]];
              case AuthenticationInApp::StateFallbackInBrowser:
                REPORTERROR(ErrorHandler::AuthenticationError, name());
                maybeComplete(false);
                break;

              default:
                // All the other states should be handled by the
                // front-end code.
                break;
            }
          });

  m_authenticationInAppSession->start(this, pkceCodeChallenge,
                                      CODE_CHALLENGE_METHOD,
                                      MozillaVPN::instance()->user()->email());
}
