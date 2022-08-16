/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <QObject>
#include <QQmlComponent>

class Navigator final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Navigator)

  Q_PROPERTY(QQmlComponent* component MEMBER m_currentComponent NOTIFY
                 currentComponentChanged)

 public:
  enum Screen {
    ScreenInitialize,
    ScreenAuthenticationInApp,
    ScreenAuthenticating,
    ScreenPostAuthentication,
    ScreenSubscriptionNeeded,
    ScreenSubscriptionInProgress,
    ScreenSubscriptionBlocked,
    ScreenSubscriptionNotValidated,
    ScreenTelemetryPolicy,
    ScreenUpdateRequired,
    ScreenHome,
    ScreenMessaging,
    ScreenSettings,
    ScreenContactUs,
    ScreenGetHelp,
    ScreenViewLogs,
  };
  Q_ENUM(Screen);

  static Navigator* instance();

  ~Navigator();

  Q_INVOKABLE void requestScreen(Screen screen);
  Q_INVOKABLE void requestPreviousScreen();

 signals:
  void currentComponentChanged();

 private:
  explicit Navigator(QObject* parent);

  void computeComponent();
  void loadScreen(Screen screen, QQmlComponent* component);

 private:
  QQmlComponent* m_currentComponent = nullptr;
  QList<Screen> m_screenHistory;
};

#endif  // NAVIGATOR_H
