/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef AUTHENTICATIONINAPP_H
#define AUTHENTICATIONINAPP_H

#include <QObject>
#include <QUrl>

class AuthenticationInAppListener;

class AuthenticationInApp final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AuthenticationInApp)

  Q_PROPERTY(int verificationCodeLength READ verificationCodeLength CONSTANT);
  Q_PROPERTY(QString emailAddress READ emailAddress NOTIFY emailAddressChanged);

 public:
  enum State {
    // The Authencation-In-App has not started yet
    StateInitializing,
    // The client_id and other params has been received. We are ready to
    // receive email address
    StateStart,
    // The email address is checked.
    StateCheckingAccount,
    // Sign in
    StateSignIn,
    // Sign up
    StateSignUp,
    // Signing-in
    StateSigningIn,
    // Signing-up
    StateSigningUp,
    // The authentication requires an unblock code (6-digit code) At this
    // point, the user needs to check their mailbox and pass the 6-digit
    // unblock code. Then, the signIn() can continue.  The code expires after 5
    // minutes. Call `resendUnblockCodeEmail` to have a new code.
    StateUnblockCodeNeeded,
    // Verification in progress,
    StateVerifyingUnblockCode,
    // The authentication requires an account verification (6-digit code) This
    // is similar to the previous step, but it happens when the account has not
    // been verified yet.  The code expires after 5 minutes. Call
    // `resendVerificationSessionCodeEmail` to have a new code.
    StateVerificationSessionByEmailNeeded,
    // Verification in progress
    StateVerifyingSessionEmailCode,
    // The two-factor authentication session verification.
    StateVerificationSessionByTotpNeeded,
    // Verification in progress
    StateVerifyingSessionTotpCode,
    // If we are unable to continue the authentication in-app, the fallback is
    // the browser flow.
    StateFallbackInBrowser,
  };
  Q_ENUM(State);

  enum ErrorType {
    ErrorAccountAlreadyExists,
    ErrorEmailAlreadyExists,
    ErrorEmailCanNotBeUsedToLogin,
    ErrorEmailTypeNotSupported,
    ErrorFailedToSendEmail,
    ErrorIncorrectPassword,
    ErrorInvalidEmailAddress,
    ErrorInvalidEmailCode,
    ErrorInvalidOrExpiredVerificationCode,
    ErrorInvalidUnblockCode,
    ErrorInvalidTotpCode,
    ErrorTooManyRequests,
    ErrorServerUnavailable,
    ErrorUnknownAccount,
  };
  Q_ENUM(ErrorType);

 private:
  Q_PROPERTY(State state READ state NOTIFY stateChanged)

 public:
  static AuthenticationInApp* instance();

  ~AuthenticationInApp();

  State state() const { return m_state; }

  // Everything starts from here.
  Q_INVOKABLE void checkAccount(const QString& emailAddress);

  Q_INVOKABLE void setPassword(const QString& password);

  Q_INVOKABLE static bool validateEmailAddress(const QString& emailAddress);

  Q_INVOKABLE static bool validatePasswordCommons(const QString& password);
  Q_INVOKABLE static bool validatePasswordLength(const QString& password);
  Q_INVOKABLE bool validatePasswordEmail(const QString& password);

  // This method restarts the authentication flow and sets the state to
  // StateStart.
  Q_INVOKABLE void reset();

  // Sign In/Up.
  Q_INVOKABLE void signIn();
  Q_INVOKABLE void signUp();

#ifdef UNIT_TEST
  // This method is used to have a test coverage for the TOTP verification.
  void enableTotpCreation();
  void allowUpperCaseEmailAddress();
#endif

  // This needs to be called when we are in StateUnblockCodeNeeded state.
  Q_INVOKABLE void verifyUnblockCode(const QString& unblockCode);

  // This can be called when we are in StateUnblockCodeNeeded state.
  Q_INVOKABLE void resendUnblockCodeEmail();

  // This needs to be called when we are in
  // StateVerificationSessionByEmailNeeded state.
  Q_INVOKABLE void verifySessionEmailCode(const QString& code);

  // This needs to be called when we are in
  // StateVerificationSessionByEmailNeeded state.
  Q_INVOKABLE void resendVerificationSessionCodeEmail();

  // This needs to be called when we are in
  // StateVerificationSessionByTotpNeeded state.
  Q_INVOKABLE void verifySessionTotpCode(const QString& code);

  void registerListener(AuthenticationInAppListener* listener);

  void requestEmailAddressChange(AuthenticationInAppListener* listener);
  void requestState(State state, AuthenticationInAppListener* listener);
  void requestErrorPropagation(ErrorType errorType,
                               AuthenticationInAppListener* listener);

  static int verificationCodeLength() { return 6; }

  const QString& emailAddress() const;

 signals:
  void stateChanged();

  void errorOccurred(ErrorType error);

  void emailAddressChanged();

#ifdef UNIT_TEST
  void unitTestFinalUrl(const QUrl& url);
  void unitTestTotpCodeCreated(const QByteArray& data);
#endif

 private:
  explicit AuthenticationInApp(QObject* parent);

  void setState(State state);

 private:
  State m_state = StateInitializing;

  AuthenticationInAppListener* m_listener = nullptr;
};

#endif  // AUTHENTICATIONINAPP_H
