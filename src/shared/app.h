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

 public:
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

  UserState userState() const;
  void setUserState(UserState userState);

  static bool isUserAuthenticated() {
    return App::instance()->userState() == App::UserAuthenticated;
  }

  static QByteArray authorizationHeader();

 signals:
  void userStateChanged();

 protected:
  App(QObject* parent);

 private:
  UserState m_userState = UserNotAuthenticated;
};

#endif  // APP_H
