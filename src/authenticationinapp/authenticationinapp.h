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

 public:
  enum State {
    // The AIP has not started yet
    StateInitializing,
    // The client_id and other params has been received. We are ready to
    // receive email address and password.
    StateStart,
    // Sign in
    StateSignIn,
    // Sign up
    StateSignUp,
    // The authentication requires an unblock code (6-digit code) At this
    // point, the user needs to check their mailbox and pass the 6-digit
    // unblock code. Then, the signIn() can continue.  The code expires after 5
    // minutes. Call `resendUnblockCodeEmail` to have a new code.
    StateUnblockCodeNeeded,
    // The authentication requires an account verification (6-digit code) This
    // is similar to the previous step, but it happens when the account has not
    // been verified yet.  The code expires after 5 minutes. Call
    // `resendVerificationSessionCodeEmail` to have a new code.
    StateVerificationSessionByEmailNeeded,
    // The two-factor authentication session verification.
    StateVerificationSessionByTotpNeeded,
    // If we are unable to continue the authentication in-app, the fallback is
    // the browser flow.
    StateFallbackInBrowser,
  };
  Q_ENUM(State);

  enum ErrorType {
    ErrorAccountAlreadyExists,
    ErrorUnknownAccount,
    ErrorIncorrectPassword,
    ErrorInvalidParameter,
    ErrorInvalidEmailCode,
    ErrorEmailTypeNotSupported,
    ErrorEmailAlreadyExists,
    ErrorEmailCanNotBeUsedToLogin,
    ErrorFailedToSendEmail,
    ErrorTooManyRequests,
    ErrorInvalidPhoneNumber,
    ErrorInvalidRegion,
    ErrorServerUnavailable,
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

  // Sign In/Up.
  Q_INVOKABLE void signIn();
  Q_INVOKABLE void signUp();

  // This needs to be called when we are in StateUnblockCodeNeeded state.
  Q_INVOKABLE void setUnblockCodeAndContinue(const QString& unblockCode);

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

  void requestState(State state, AuthenticationInAppListener* listener);
  void requestErrorPropagation(ErrorType errorType,
                               AuthenticationInAppListener* listener);

 signals:
  void stateChanged();

  void errorOccurred(ErrorType error);

 private:
  explicit AuthenticationInApp(QObject* parent);

  void setState(State state);

 private:
  State m_state = StateInitializing;

  AuthenticationInAppListener* m_listener = nullptr;
};

#endif  // AUTHENTICATIONINAPP_H
