/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <QObject>
#include <QQmlComponent>

class NavigatorReloader;
class QQuickItem;

class Navigator final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Navigator)

  Q_PROPERTY(
      Screen screen MEMBER m_currentScreen NOTIFY currentComponentChanged)
  Q_PROPERTY(LoadPolicy loadPolicy MEMBER m_currentLoadPolicy NOTIFY
                 currentComponentChanged)
  Q_PROPERTY(LoadingFlags loadingFlags MEMBER m_currentLoadingFlags NOTIFY
                 currentComponentChanged)
  Q_PROPERTY(QQmlComponent* component MEMBER m_currentComponent NOTIFY
                 currentComponentChanged)

 public:
  enum LoadPolicy {
    LoadPersistently,
    LoadTemporarily,
  };
  Q_ENUM(LoadPolicy);

  enum LoadingFlags {
    NoFlags,
    ForceReload,
    ForceReloadAll,
  };
  Q_ENUM(LoadingFlags);

  enum Screen {
    ScreenAuthenticating,
    ScreenAuthenticationInApp,
    ScreenBackendFailure,
    ScreenBillingNotAvailable,
    ScreenCaptivePortal,
    ScreenCrashReporting,
    ScreenDeleteAccount,
    ScreenDeviceLimit,
    ScreenGetHelp,
    ScreenHome,
    ScreenInitialize,
    ScreenMessaging,
    ScreenNoSubscriptionFoundError,
    ScreenPostAuthentication,
    ScreenSettings,
    ScreenSubscriptionBlocked,
    ScreenSubscriptionNeededIAP,
    ScreenSubscriptionNeededWeb,
    ScreenSubscriptionExpiredError,
    ScreenSubscriptionGenericError,
    ScreenSubscriptionInProgressIAP,
    ScreenSubscriptionInProgressWeb,
    ScreenSubscriptionInUseError,
    ScreenSubscriptionNotValidated,
    ScreenTelemetryPolicy,
    ScreenTipsAndTricks,
    ScreenUpdateRecommended,
    ScreenUpdateRequired,
    ScreenViewLogs,
  };
  Q_ENUM(Screen);

  static Navigator* instance();

  ~Navigator();

  Q_INVOKABLE void requestScreen(
      Navigator::Screen screen, Navigator::LoadingFlags loadingFlags = NoFlags);
  Q_INVOKABLE void requestPreviousScreen();

  Q_INVOKABLE void addStackView(Navigator::Screen screen,
                                const QVariant& stackView);
  Q_INVOKABLE void addView(Navigator::Screen screen, const QVariant& view);

  Q_INVOKABLE bool eventHandled();

  void registerReloader(NavigatorReloader* reloader);
  void unregisterReloader(NavigatorReloader* reloader);

 signals:
  void goBack(QQuickItem* item);
  void currentComponentChanged();

 private:
  explicit Navigator(QObject* parent);

  void computeComponent();
  void loadScreen(Screen screen, LoadPolicy loadPolicy,
                  QQmlComponent* component, LoadingFlags loadingFlags);

  void removeItem(QObject* obj);

 private:
  Screen m_currentScreen = ScreenInitialize;
  LoadPolicy m_currentLoadPolicy = LoadTemporarily;
  LoadingFlags m_currentLoadingFlags = NoFlags;
  QQmlComponent* m_currentComponent = nullptr;

  QList<Screen> m_screenHistory;

  QList<NavigatorReloader*> m_reloaders;
};

#endif  // NAVIGATOR_H
