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
    // We are checking the account.
    StateAccountStatus,
    // Sign in
    StateSignIn,
    // Sign up
    StateSignUp,
    // The authentication requires an email verification (6-digit code)
    // At this point, the user is not sign-in/up. We are waiting for this extra
    // step to complete the authentication.
    StateEmailVerification,
    // The authentication requires an account verification (6-digit code)
    // This is similar to the previous step, but it happens when the account
    // has not been verified yet.
    // The code expires after 5 minutes. Call `resendVerificationAccountCode`
    // to have a new code.
    StateAccountVerification,
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

  Q_INVOKABLE void signInOrUp(const QString& emailAddress,
                              const QString& password);

  // This needs to be called when we are in StateEmailVerification state.
  Q_INVOKABLE void verifyEmailCode(const QString& code);

  // This needs to be called when we are in StateAccountVerification state.
  Q_INVOKABLE void verifyAccountCode(const QString& code);

  // This needs to be called when we are in StateAccountVerification state.
  Q_INVOKABLE void resendVerificationAccountCode();

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
