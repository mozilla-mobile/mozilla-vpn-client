/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APP_H
#define APP_H

#include <QObject>

class App : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(App)

  Q_PROPERTY(bool userAuthenticated READ userAuthenticated NOTIFY
             userAuthenticationChanged)
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

  // Important! Each app _must_ implement this static method.
  static App* instance();

  virtual ~App();

  int state() const;
  void setState(int state);

  bool userAuthenticated() const;
  static bool isUserAuthenticated() { return instance()->userAuthenticated(); };

  static QByteArray authorizationHeader();

  void quit();

 signals:
  void userAuthenticationChanged();
  void stateChanged();

 protected:
  App(QObject* parent);

 private:
  int m_state = StateInitialize;
};

#endif  // APP_H
