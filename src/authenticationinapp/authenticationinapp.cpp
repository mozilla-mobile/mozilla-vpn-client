/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "authenticationinapp.h"

#include <QCoreApplication>
#include <QFile>
#include <QMetaEnum>
#include <QRegularExpression>

#include "authenticationinappsession.h"
#include "glean/generated/metrics.h"
#include "glean/metrictypes.h"
#include "incrementaldecoder.h"
#include "leakdetector.h"
#include "logger.h"
#include "resourceloader.h"

constexpr int PASSWORD_MIN_LENGTH = 8;

namespace {
Logger logger("AuthenticationInApp");
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
  MZ_COUNT_CTOR(AuthenticationInApp);
  Q_ASSERT(!s_instance);
  s_instance = this;

  connect(ResourceLoader::instance(), &ResourceLoader::cacheFlushNeeded, this,
          [this]() { m_encodedPassword.clear(); });
}

AuthenticationInApp::~AuthenticationInApp() {
  MZ_COUNT_DTOR(AuthenticationInApp);
  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

void AuthenticationInApp::setState(State state,
                                   AuthenticationInAppSession* session) {
  m_state = state;
  emit stateChanged();
}

void AuthenticationInApp::registerSession(AuthenticationInAppSession* session) {
  Q_ASSERT(session);
  Q_ASSERT(!m_session);

  m_session = session;
  connect(session, &QObject::destroyed, this, [this]() {
    AuthenticationInAppSession* session = m_session;
    m_session = nullptr;
    setState(StateInitializing, session);
  });
}

void AuthenticationInApp::checkAccount(const QString& emailAddress) {
  Q_ASSERT(m_state == StateStart);
  Q_ASSERT(m_session);

  logger.debug() << "Authentication starting";

  m_session->checkAccount(emailAddress);
}

void AuthenticationInApp::reset() {
  Q_ASSERT(m_session);
  logger.debug() << "Authentication reset";
  setState(StateStart, m_session);
  m_session->reset();
}

void AuthenticationInApp::setPassword(const QString& password) {
  Q_ASSERT(m_state == StateSignIn || m_state == StateSignUp);
  Q_ASSERT(m_session);

  logger.debug() << "Setting the password";

  m_session->setPassword(password);
}

void AuthenticationInApp::signIn() {
#ifndef UNIT_TEST
  // In unit-test we try to reproduce errors creating race-conditions between
  // sign-in/sign-up. Basically we ignore we are in sign-in and we proceed with
  // a sign-up. This is a race-condition between 2 devices and it's not
  // possible to reproduce it with one single istance of AuthenticationInApp.
  Q_ASSERT(m_state == StateSignIn);
#endif
  Q_ASSERT(m_session);

  logger.debug() << "Sign In";

  m_session->signIn();
}

const QString& AuthenticationInApp::emailAddress() const {
  if (!m_session){
    return QString();
  }

  logger.debug() << "Get email address";

  return m_session->emailAddress();
}

const QStringList& AuthenticationInApp::attachedClients() const {
  if (!m_session) {
    return QStringList();
  }

  logger.debug() << "Get attached clients";

  return m_session->attachedClients();
}

void AuthenticationInApp::signUp() {
#ifndef UNIT_TEST
  // In unit-test we try to reproduce errors creating race-conditions between
  // sign-in/sign-up. Basically we ignore we are in sign-in and we proceed with
  // a sign-up. This is a race-condition between 2 devices and it's not
  // possible to reproduce it with one single istance of AuthenticationInApp.
  Q_ASSERT(m_state == StateSignUp);
#endif
  Q_ASSERT(m_session);

  logger.debug() << "Sign Up";

  m_session->signUp();
}

#ifdef UNIT_TEST
void AuthenticationInApp::enableTotpCreation() {
  Q_ASSERT(m_state == StateSignIn || m_state == StateSignUp);
  Q_ASSERT(m_session);

  m_session->enableTotpCreation();
}

void AuthenticationInApp::allowUpperCaseEmailAddress() {
  Q_ASSERT(m_session);
  m_session->allowUpperCaseEmailAddress();
}
#endif

void AuthenticationInApp::verifyUnblockCode(const QString& unblockCode) {
  Q_ASSERT(m_state == StateUnblockCodeNeeded);
  Q_ASSERT(m_session);
  m_session->verifyUnblockCode(unblockCode);
}

void AuthenticationInApp::resendUnblockCodeEmail() {
  Q_ASSERT(m_state == StateUnblockCodeNeeded);
  Q_ASSERT(m_session);
  m_session->sendUnblockCodeEmail();
}

void AuthenticationInApp::verifySessionEmailCode(const QString& code) {
  Q_ASSERT(m_state == StateVerificationSessionByEmailNeeded);
  Q_ASSERT(m_session);
  m_session->verifySessionEmailCode(code);
}

void AuthenticationInApp::resendVerificationSessionCodeEmail() {
  Q_ASSERT(m_state == StateVerificationSessionByEmailNeeded);
  Q_ASSERT(m_session);
  m_session->resendVerificationSessionCodeEmail();
}

void AuthenticationInApp::verifySessionTotpCode(const QString& code) {
  Q_ASSERT(m_state == StateVerificationSessionByTotpNeeded);
  Q_ASSERT(m_session);
  m_session->verifySessionTotpCode(code);
}

void AuthenticationInApp::deleteAccount() {
  Q_ASSERT(m_state == StateAccountDeletionRequest);
  Q_ASSERT(m_session);
  m_session->deleteAccount();
}

void AuthenticationInApp::requestEmailAddressChange(
    AuthenticationInAppSession* session) {
  Q_ASSERT(session);
  Q_ASSERT(m_session == session);
  emit emailAddressChanged();
}

void AuthenticationInApp::requestAttachedClientsChange(
    AuthenticationInAppSession* session) {
  Q_ASSERT(session);
  Q_ASSERT(m_session == session);
  emit attachedClientsChanged();
}

void AuthenticationInApp::requestState(State state,
                                       AuthenticationInAppSession* session) {
  Q_ASSERT(session);
  Q_ASSERT(m_session == session);

  setState(state, m_session);
}

void AuthenticationInApp::requestErrorPropagation(
    AuthenticationInAppSession* session, ErrorType errorType,
    uint32_t retryAfterSec) {
  Q_ASSERT(session);
  Q_ASSERT(m_session == session);

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

  static QRegularExpression emailRE("^[A-Z0-9.!#$%&'*+/=?^_`{|}~-]{1,64}$",
                                    QRegularExpression::CaseInsensitiveOption);
  // We don't have to convert the first part of the email address to ASCII
  // Compatible Encoding (ace).
  if (!emailRE.match(parts[0]).hasMatch()) {
    return false;
  }

  QByteArray domainAce = QUrl::toAce(parts[1]);
  static QRegularExpression domainRE(
      "^[A-Z0-9](?:[A-Z0-9-]{0,253}[A-Z0-9])?(?:\\.[A-Z0-9](?:[A-Z0-9-]{0,253}["
      "A-Z0-9])?)+$",
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
    QFile file(ResourceLoader::instance()->loadFile(
        ":/resources/encodedPassword.txt"));
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
  Q_ASSERT(m_session);

  return !m_session->emailAddress().contains(password);
}

void AuthenticationInApp::terminateSession() {
  if (m_session) {
    m_session->terminate();
  }
}
