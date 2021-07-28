/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "authenticationinapp.h"
#include "authenticationinapplistener.h"
#include "logger.h"
#include "leakdetector.h"

#include <QCoreApplication>

namespace {
Logger logger(LOG_MAIN, "AuthenticationInApp");
AuthenticationInApp* s_instance = nullptr;
}  // namespace

// static
AuthenticationInApp* AuthenticationInApp::instance() {
  if (!s_instance) {
    new AuthenticationInApp(qApp);
  }
  Q_ASSERT(s_instance);
  return s_instance;
}

AuthenticationInApp::AuthenticationInApp(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(AuthenticationInApp);
  Q_ASSERT(!s_instance);
  s_instance = this;
}

AuthenticationInApp::~AuthenticationInApp() {
  MVPN_COUNT_DTOR(AuthenticationInApp);
  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

void AuthenticationInApp::setState(State state) {
  m_state = state;
  emit stateChanged();
}

void AuthenticationInApp::registerListener(
    AuthenticationInAppListener* listener) {
  Q_ASSERT(listener);
  Q_ASSERT(!m_listener);

  m_listener = listener;
  connect(listener, &QObject::destroyed, [this]() {
    m_listener = nullptr;
    setState(StateInitializing);
  });
}

void AuthenticationInApp::signInOrUp(const QString& emailAddress,
                                     const QString& password) {
  Q_ASSERT(m_state == StateStart);
  Q_ASSERT(m_listener);

  logger.log() << "Authentication starting:" << emailAddress;

  m_listener->signInOrUp(emailAddress, password);
}

void AuthenticationInApp::verifyEmailCode(const QString& code) {
  Q_ASSERT(m_state == StateEmailVerification);
  Q_ASSERT(m_listener);
  m_listener->verifyEmailCode(code);
}

void AuthenticationInApp::verifyAccountCode(const QString& code) {
  Q_ASSERT(m_state == StateAccountVerification);
  Q_ASSERT(m_listener);
  m_listener->verifyAccountCode(code);
}

void AuthenticationInApp::resendVerificationAccountCode() {
  Q_ASSERT(m_state == StateAccountVerification);
  Q_ASSERT(m_listener);
  m_listener->resendVerificationAccountCode();
}

void AuthenticationInApp::requestState(State state,
                                       AuthenticationInAppListener* listener) {
  Q_ASSERT(listener);
  Q_ASSERT(m_listener == listener);

  setState(state);
}

void AuthenticationInApp::requestErrorPropagation(
    ErrorType errorType, AuthenticationInAppListener* listener) {
  Q_ASSERT(listener);
  Q_ASSERT(m_listener == listener);

  emit errorOccurred(errorType);
}
