/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <QObject>
#include <QQmlComponent>

class QQuickItem;

class Navigator final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Navigator)

  Q_PROPERTY(
      Screen screen MEMBER m_currentScreen NOTIFY currentComponentChanged)
  Q_PROPERTY(LoadPolicy loadPolicy MEMBER m_currentLoadPolicy NOTIFY
                 currentComponentChanged)
  Q_PROPERTY(QQmlComponent* component MEMBER m_currentComponent NOTIFY
                 currentComponentChanged)

 public:
  enum LoadPolicy {
    LoadPersistently,
    LoadTemporarily,
  };
  Q_ENUM(LoadPolicy);

  enum Screen {
    ScreenAuthenticating,
    ScreenAuthenticationInApp,
    ScreenBackendFailure,
    ScreenBillingNotAvailable,
    ScreenCaptivePortal,
    ScreenDeviceLimit,
    ScreenGetHelp,
    ScreenHome,
    ScreenInitialize,
    ScreenMessaging,
    ScreenNoSubscriptionFoundError,
    ScreenPostAuthentication,
    ScreenSettings,
    ScreenSubscriptionBlocked,
    ScreenSubscriptionNeeded,
    ScreenSubscriptionExpiredError,
    ScreenSubscriptionGenericError,
    ScreenSubscriptionInProgress,
    ScreenSubscriptionInUseError,
    ScreenSubscriptionNotValidated,
    ScreenTelemetryPolicy,
    ScreenUpdateRequired,
    ScreenViewLogs,
  };
  Q_ENUM(Screen);

  static Navigator* instance();

  ~Navigator();

  Q_INVOKABLE void requestScreen(Screen screen);
  Q_INVOKABLE void requestPreviousScreen();

  Q_INVOKABLE void addStackView(Screen screen, const QVariant& stackView);
  Q_INVOKABLE void addView(Screen screen, const QVariant& view);

  Q_INVOKABLE bool eventHandled();

 signals:
  void goBack(QQuickItem* item);
  void currentComponentChanged();

 private:
  explicit Navigator(QObject* parent);

  void computeComponent();
  void loadScreen(Screen screen, LoadPolicy loadPolicy,
                  QQmlComponent* component);

  void removeItem(QObject* obj);

 private:
  Screen m_currentScreen = ScreenInitialize;
  LoadPolicy m_currentLoadPolicy = LoadTemporarily;
  QQmlComponent* m_currentComponent = nullptr;

  QList<Screen> m_screenHistory;
};

#endif  // NAVIGATOR_H
