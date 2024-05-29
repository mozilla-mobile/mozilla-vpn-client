/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APP_H
#define APP_H

#include <QObject>

class App : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(App)

  Q_PROPERTY(UserState userState READ userState NOTIFY userStateChanged)
  Q_PROPERTY(bool userAuthenticated READ userAuthenticated NOTIFY userStateChanged)
  Q_PROPERTY(int state READ state NOTIFY stateChanged)

 public:
  enum State : int {
    // This is the first state when the app starts. During the initialization,
    // the app can move to a different state
    StateInitialize = 0,

    // The authentication flow has started.
    StateAuthenticating,

    // Something went wrong during the purchase initialization.
    StateBillingNotAvailable,

    // he app is fully activated, and the user is authenticated and subscribed.
    // All good!
    StateMain,

    // Something went wrong during the subscription flow. We are unable to
    // complete the subscription.
    StateSubscriptionBlocked,

    // The user is authenticated but has yet to be subscribed.
    StateSubscriptionNeeded,

    // The subscription flow is in progress.
    StateSubscriptionInProgress,

    // An error occurred during the subscription validation.
    StateSubscriptionNotValidated,

    // The user was successfully authenticated and moves into the onboarding
    // flow
    StateOnboarding,

    // This is a sentinel value. Any extra state can be configured using values
    // greater than this.
    StateCustom = 1000,
  };
  Q_ENUM(State);

  enum UserState {
    // The user is not authenticated and there is not a logging-out operation
    // in progress. Maybe we are running the authentication flow (to know if we
    // are running the authentication flow, please use the
    // `StateAuthenticating` state).
    UserNotAuthenticated,

    // The user is authenticated and there is not a logging-out operation in
    // progress.
    UserAuthenticated,

    // We are logging out the user. There are a few steps to run in order to
    // complete the logout. In the meantime, the user should be considered as
    // not-authenticated. The next state will be `UserNotAuthenticated`.
    UserLoggingOut,
  };
  Q_ENUM(UserState);

  // Important! Each app _must_ implement this static method.
  static App* instance();

  virtual ~App();

  int state() const;
  void setState(int state);

  UserState userState() const;
  void setUserState(UserState userState);

  bool userAuthenticated() const { return m_userState == UserAuthenticated; };
  static bool isUserAuthenticated() { return instance()->userAuthenticated(); };

  static QByteArray authorizationHeader();

  void quit();

 signals:
  void stateChanged();
  void userStateChanged();

 protected:
  App(QObject* parent);

 private:
  int m_state = StateInitialize;
  UserState m_userState = UserNotAuthenticated;
};

#endif  // APP_H
