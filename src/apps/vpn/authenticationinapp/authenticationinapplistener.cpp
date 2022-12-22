/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "authenticationinapplistener.h"

#include "authenticationinappsession.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("AuthenticationInAppListener");
}  // anonymous namespace

AuthenticationInAppListener::AuthenticationInAppListener(QObject* parent)
    : AuthenticationListener(parent) {
  MZ_COUNT_CTOR(AuthenticationInAppListener);

  m_session = new AuthenticationInAppSession(
      this, AuthenticationInAppSession::TypeDefault);
  connect(m_session, &AuthenticationInAppSession::completed, this,
          &AuthenticationListener::completed);
  connect(m_session, &AuthenticationInAppSession::failed, this,
          &AuthenticationListener::failed);
  connect(m_session, &AuthenticationInAppSession::terminated, this,
          &AuthenticationListener::readyToFinish);
  connect(m_session, &AuthenticationInAppSession::fallbackRequired, this,
          &AuthenticationInAppListener::fallbackRequired);
}

AuthenticationInAppListener::~AuthenticationInAppListener() {
  MZ_COUNT_DTOR(AuthenticationInAppListener);
}

void AuthenticationInAppListener::start(Task* task,
                                        const QString& codeChallenge,
                                        const QString& codeChallengeMethod,
                                        const QString& emailAddress) {
  logger.debug() << "AuthenticationInAppListener initialized";

  m_task = task;
  m_codeChallenge = codeChallenge;
  m_codeChallengeMethod = codeChallengeMethod;

  m_session->start(task, codeChallenge, codeChallengeMethod, emailAddress);
}

void AuthenticationInAppListener::aboutToFinish() {
  logger.debug() << "About to finish";
  m_session->terminate();
}

void AuthenticationInAppListener::fallbackRequired() {
  logger.debug() << "Fallback required";

  AuthenticationListener* fallbackListener =
      create(this, MozillaVPN::AuthenticationInBrowser);
  fallbackListener->start(m_task, m_codeChallenge, m_codeChallengeMethod,
                          m_session->emailAddress());

  connect(fallbackListener, &AuthenticationListener::completed, this,
          &AuthenticationListener::completed);
  connect(fallbackListener, &AuthenticationListener::failed, this,
          &AuthenticationListener::failed);
  connect(fallbackListener, &AuthenticationListener::abortedByUser, this,
          &AuthenticationListener::abortedByUser);
}
