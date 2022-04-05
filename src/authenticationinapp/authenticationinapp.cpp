/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "authenticationinapp.h"
#include "authenticationinapplistener.h"
#include "logger.h"
#include "leakdetector.h"
#include "incrementaldecoder.h"

#include "../../glean/telemetry/gleansample.h"

#include <QCoreApplication>
#include <QFile>
#include <QMetaEnum>
#include <QRegularExpression>

constexpr int PASSWORD_MIN_LENGTH = 8;

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

  emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
      GleanSample::authenticationInappStep,
      {{"state", QVariant::fromValue(state).toString()}});
}

void AuthenticationInApp::registerListener(
    AuthenticationInAppListener* listener) {
  Q_ASSERT(listener);
  Q_ASSERT(!m_listener);

  m_listener = listener;
  connect(listener, &QObject::destroyed, this, [this]() {
    m_listener = nullptr;
    setState(StateInitializing);
  });
}

void AuthenticationInApp::checkAccount(const QString& emailAddress) {
  Q_ASSERT(m_state == StateStart);
  Q_ASSERT(m_listener);

  logger.debug() << "Authentication starting";

  m_listener->checkAccount(emailAddress);
}

void AuthenticationInApp::reset() {
  Q_ASSERT(m_listener);
  logger.debug() << "Authentication reset";
  setState(StateStart);
  m_listener->reset();
}

void AuthenticationInApp::setPassword(const QString& password) {
  Q_ASSERT(m_state == StateSignIn || m_state == StateSignUp);
  Q_ASSERT(m_listener);

  logger.debug() << "Setting the password";

  m_listener->setPassword(password);
}

void AuthenticationInApp::signIn() {
#ifndef UNIT_TEST
  // In unit-test we try to reproduce errors creating race-conditions between
  // sign-in/sign-up. Basically we ignore we are in sign-in and we proceed with
  // a sign-up. This is a race-condition between 2 devices and it's not
  // possible to reproduce it with one single istance of AuthenticationInApp.
  Q_ASSERT(m_state == StateSignIn);
#endif
  Q_ASSERT(m_listener);

  logger.debug() << "Sign In";

  m_listener->signIn();
}

const QString& AuthenticationInApp::emailAddress() const {
  Q_ASSERT(m_listener);

  logger.debug() << "Get email address";

  return m_listener->emailAddress();
}

void AuthenticationInApp::signUp() {
#ifndef UNIT_TEST
  // In unit-test we try to reproduce errors creating race-conditions between
  // sign-in/sign-up. Basically we ignore we are in sign-in and we proceed with
  // a sign-up. This is a race-condition between 2 devices and it's not
  // possible to reproduce it with one single istance of AuthenticationInApp.
  Q_ASSERT(m_state == StateSignUp);
#endif
  Q_ASSERT(m_listener);

  logger.debug() << "Sign Up";

  m_listener->signUp();
}

#ifdef UNIT_TEST
void AuthenticationInApp::enableTotpCreation() {
  Q_ASSERT(m_state == StateSignIn || m_state == StateSignUp);
  Q_ASSERT(m_listener);

  m_listener->enableTotpCreation();
}

void AuthenticationInApp::allowUpperCaseEmailAddress() {
  Q_ASSERT(m_listener);
  m_listener->allowUpperCaseEmailAddress();
}

void AuthenticationInApp::enableAccountDeletion() {
  Q_ASSERT(m_state == StateSignIn || m_state == StateSignUp);
  Q_ASSERT(m_listener);

  m_listener->enableAccountDeletion();
}
#endif

void AuthenticationInApp::verifyUnblockCode(const QString& unblockCode) {
  Q_ASSERT(m_state == StateUnblockCodeNeeded);
  Q_ASSERT(m_listener);
  m_listener->verifyUnblockCode(unblockCode);
}

void AuthenticationInApp::resendUnblockCodeEmail() {
  Q_ASSERT(m_state == StateUnblockCodeNeeded);
  Q_ASSERT(m_listener);
  m_listener->sendUnblockCodeEmail();
}

void AuthenticationInApp::verifySessionEmailCode(const QString& code) {
  Q_ASSERT(m_state == StateVerificationSessionByEmailNeeded);
  Q_ASSERT(m_listener);
  m_listener->verifySessionEmailCode(code);
}

void AuthenticationInApp::resendVerificationSessionCodeEmail() {
  Q_ASSERT(m_state == StateVerificationSessionByEmailNeeded);
  Q_ASSERT(m_listener);
  m_listener->resendVerificationSessionCodeEmail();
}

void AuthenticationInApp::verifySessionTotpCode(const QString& code) {
  Q_ASSERT(m_state == StateVerificationSessionByTotpNeeded);
  Q_ASSERT(m_listener);
  m_listener->verifySessionTotpCode(code);
}

void AuthenticationInApp::requestEmailAddressChange(
    AuthenticationInAppListener* listener) {
  Q_ASSERT(listener);
  Q_ASSERT(m_listener == listener);
  emit emailAddressChanged();
}

void AuthenticationInApp::requestState(State state,
                                       AuthenticationInAppListener* listener) {
  Q_ASSERT(listener);
  Q_ASSERT(m_listener == listener);

  setState(state);
}

void AuthenticationInApp::requestErrorPropagation(
    AuthenticationInAppListener* listener, ErrorType errorType,
    uint32_t retryAfterSec) {
  Q_ASSERT(listener);
  Q_ASSERT(m_listener == listener);

  emit errorOccurred(errorType, retryAfterSec);
}

// static
bool AuthenticationInApp::validateEmailAddress(const QString& emailAddress) {
  // https://github.com/mozilla/fxa/blob/main/packages/fxa-auth-server/lib/routes/validators.js#L144-L177

  if (emailAddress.isEmpty()) {
    return false;
  }

  QStringList parts = emailAddress.split("@");
  if (parts.length() != 2 || parts[1].length() > 255) {
    return false;
  }

  QRegularExpression emailRE("^[A-Z0-9.!#$%&'*+/=?^_`{|}~-]{1,64}$",
                             QRegularExpression::CaseInsensitiveOption);
  // We don't have to convert the first part of the email address to ASCII
  // Compatible Encoding (ace).
  if (!emailRE.match(parts[0]).hasMatch()) {
    return false;
  }

  QByteArray domainAce = QUrl::toAce(parts[1]);
  QRegularExpression domainRE(
      "^[A-Z0-9](?:[A-Z0-9-]{0,253}[A-Z0-9])?(?:.[A-Z0-9](?:[A-Z0-9-]{0,253}[A-"
      "Z0-9])?)+$",
      QRegularExpression::CaseInsensitiveOption);
  if (!domainRE.match(domainAce).hasMatch()) {
    return false;
  }

  return true;
}

bool AuthenticationInApp::validatePasswordCommons(const QString& password) {
  if (!validatePasswordLength(password)) {
    // The task of this function is not the length validation.
    return true;
  }

  // Let's cache the encoded-password content.
  if (m_encodedPassword.isEmpty()) {
    QFile file(":/ui/resources/encodedPassword.txt");
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
      logger.error() << "Failed to open the encodedPassword.txt";
      return true;
    }

    m_encodedPassword = file.readAll();
  }

  QTextStream stream(&m_encodedPassword);

  IncrementalDecoder id(qApp);
  IncrementalDecoder::Result result = id.match(stream, password);
  switch (result) {
    case IncrementalDecoder::MatchNotFound:
      return true;
    case IncrementalDecoder::MatchFound:
      logger.info() << "Unsecure password";
      return false;
    case IncrementalDecoder::DecodeFailure:
      logger.error() << "Decode failure!";
      return true;
    default:
      Q_ASSERT(false);
      return true;
  }
}

// static
bool AuthenticationInApp::validatePasswordLength(const QString& password) {
  return password.length() >= PASSWORD_MIN_LENGTH;
}

bool AuthenticationInApp::validatePasswordEmail(const QString& password) {
  Q_ASSERT(m_state == StateSignUp);
  Q_ASSERT(m_listener);

  return !m_listener->emailAddress().contains(password);
}
