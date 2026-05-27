/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLAVPN_H
#define MOZILLAVPN_H

#include <QList>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>

#include "authenticationlistener.h"
#include "frontend/navigationbarbutton.h"
#include "frontend/navigator.h"

struct MozillaVPNPrivate;
class CaptivePortal;
class CaptivePortalDetection;
class ConnectionHealth;
class Controller;
class DeviceModel;
class IpAddressLookup;
class Keys;
class Location;
class NetworkWatcher;
class ReleaseMonitor;
class ServerCountryModel;
class ServerData;
class ServerLatency;
class StatusIcon;
class SubscriptionData;
class SupportCategoryModel;
class User;
class QEvent;
class QUrl;

class MozillaVPN final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(MozillaVPN)

  Q_PROPERTY(int state READ state NOTIFY stateChanged)
  Q_PROPERTY(bool userAuthenticated READ userAuthenticated NOTIFY
                 userAuthenticationMaybeChanged)
  Q_PROPERTY(bool startMinimized READ startMinimized CONSTANT)
  Q_PROPERTY(bool updating READ updating NOTIFY updatingChanged)

 public:
  enum State : int {
    // This is the first state when the app starts. During the initialization,
    // the app can move to a different state
    StateInitialize = 0,

    // The authentication flow has started.
    StateAuthenticating,

    // Something went wrong during the purchase initialization.
    StateBillingNotAvailable,

    // The app is fully activated, and the user is authenticated and subscribed.
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

    StateDeviceLimit,
    StateHeartbeatFailure,
    StateUpdateRequired,
    StatePermissionRequired,
  };
  Q_ENUM(State);

  // Important! Each app _must_ implement this static method.
  static MozillaVPN* instance();

  int state() const;
  void setState(int state);

  bool userAuthenticated() const;
  static bool isUserAuthenticated() { return instance()->userAuthenticated(); };

  void quit();

  enum CustomScreen {
    ScreenAuthenticating = Navigator::ScreenCustom + 1,
    ScreenAuthenticationInApp,
    ScreenHeartbeatFailure,
    ScreenBillingNotAvailable,
    ScreenCaptivePortal,
    ScreenCrashReporting,
    ScreenDeviceLimit,
    ScreenGetHelp,
    ScreenHome,
    ScreenInitialize,
    ScreenMessaging,
    ScreenNoSubscriptionFoundError,
    ScreenSettings,
    ScreenSubscriptionBlocked,
    ScreenSubscriptionExpiredError,
    ScreenSubscriptionGenericError,
    ScreenSubscriptionInProgressIAP,
    ScreenSubscriptionInProgressWeb,
    ScreenSubscriptionInUseError,
    ScreenSubscriptionNeeded,
    ScreenSubscriptionNotValidated,
    ScreenUpdateRecommended,
    ScreenUpdateRequired,
    ScreenViewLogs,
    ScreenRemovingDevice,
    ScreenOnboarding,
    ScreenPermissionRequired,
  };
  Q_ENUM(CustomScreen);

  MozillaVPN();
  ~MozillaVPN();

  // This is exactly like the ::instance() method, but it doesn't crash if the
  // MozillaVPN is null. It should be used rarely.
  static MozillaVPN* maybeInstance();

  void initialize();

  // Exposed QML methods:
  Q_INVOKABLE void authenticate();
  Q_INVOKABLE void cancelAuthentication();
  Q_INVOKABLE void removeDeviceFromPublicKey(const QString& publicKey);
  Q_INVOKABLE void onboardingCompleted();
  Q_INVOKABLE void mainWindowLoaded();
  Q_INVOKABLE void activate();
  Q_INVOKABLE void deactivate(bool block = false);
  Q_INVOKABLE void refreshDevices();
  Q_INVOKABLE void update();
  Q_INVOKABLE void triggerHeartbeat();
  Q_INVOKABLE void createSupportTicket(const QString& email,
                                       const QString& subject,
                                       const QString& issueText,
                                       const QString& category,
                                       const bool shareLogs);
  Q_INVOKABLE bool validateUserDNS(const QString& dns) const;
  Q_INVOKABLE void hardResetAndQuit();
  Q_INVOKABLE void cancelReauthentication();

  void authenticateWithType(
      AuthenticationListener::AuthenticationType authenticationType);

  // Private object getters:
  CaptivePortal* captivePortal() const;
  CaptivePortalDetection* captivePortalDetection() const;
  ConnectionHealth* connectionHealth() const;
  Controller* controller() const;
  ServerData* serverData() const;
  DeviceModel* deviceModel() const;
  IpAddressLookup* ipAddressLookup() const;
  SupportCategoryModel* supportCategoryModel() const;
  Keys* keys() const;
  Location* location() const;
  NetworkWatcher* networkWatcher() const;
  ReleaseMonitor* releaseMonitor() const;
  ServerCountryModel* serverCountryModel() const;
  ServerLatency* serverLatency() const;
  StatusIcon* statusIcon() const;
  SubscriptionData* subscriptionData() const;
  User* user() const;

  // Called at the end of the authentication flow. We can continue adding the
  // device if it doesn't exist yet, or we can go to OFF state.
  void completeAuthentication(const QByteArray& json, const QString& token);

  void deviceAdded(const QString& deviceName, const QString& publicKey,
                   const QString& privateKey);

  void removeDevice(const QString& publicKey, const QString& source);
  void deviceRemovalCompleted(const QString& publicKey);

  void serversFetched(const QByteArray& serverData);

  void accountChecked(const QByteArray& json);

  void abortAuthentication();

  Q_INVOKABLE void silentSwitch();

  Q_INVOKABLE void logout();

  bool startMinimized() const { return m_startMinimized; }

  void setStartMinimized(bool startMinimized) {
    m_startMinimized = startMinimized;
  }

  [[nodiscard]] bool setServerList(const QByteArray& serverData);

  Q_INVOKABLE void reset(bool forceInitialState);

  bool loadModels();
  bool modelsInitialized() const;
  bool hasToken() const;

  bool updating() const { return m_updating; }
  void setUpdating(bool updating);

  void heartbeatCompleted(bool success);

  void addCurrentDeviceAndRefreshData();

  void createTicketAnswerRecieved(bool successful) {
    emit ticketCreationAnswer(successful);
  }

  void requestAbout();

  static QString appVersionForUpdate();
  static bool mockFreeTrial();

  static int runCommandLineApp(std::function<int()>&& a_callback);
  static int runGuiApp(std::function<int()>&& a_callback);

  void handleDeepLink(const QUrl& url);

 signals:
  void userAuthenticationMaybeChanged();
  void stateChanged();

  void deviceRemoving(const QString& publicKey);
  void deviceRemoved(const QString& source);
  void aboutNeeded();
  void updatingChanged();
  void accountDeleted();

  void authenticationStarted();
  void authenticationAborted();
  void authenticationCompleted();

  void logSubscriptionCompleted();

  void aboutToQuit();

  void ticketCreationAnswer(bool successful);

 protected:
#if MZ_MACOS
  bool eventFilter(QObject* obj, QEvent* event);
#endif

 private:
  void hardReset();
  void maybeStateMain();

  void startSchedulingPeriodicOperations();

  void stopSchedulingPeriodicOperations();

  void subscriptionStarted(const QString& productIdentifier);
  void restoreSubscriptionStarted();
  void subscriptionCompleted();
  void subscriptionFailed();
  void subscriptionCanceled();
  void subscriptionFailedInternal(bool canceledByUser);
  void alreadySubscribed();
  void billingNotAvailable();
  void subscriptionNotValidated();

  void completeActivation();

  enum RemovalDeviceOption {
    DeviceNotFound,
    DeviceStillValid,
    DeviceRemoved,
  };

  RemovalDeviceOption maybeRemoveCurrentDevice();

  void controllerStateChanged();

  void maybeConnectOnStartup();

  void maybeRegenerateDeviceKey();

  bool checkCurrentDevice();

  void errorHandled();

  void scheduleRefreshDataTasks();

  static void registerUrlOpenerLabels();

  static void registerErrorHandlers();

  static void registerNavigatorScreens();

  static void registerInspectorCommands();

  static void registerNavigationBarButtons();

  static void setupMessageNotificationWatch(NavigationBarButton& messageIcon);

  static void registerAddonApis();

 private:
  int m_state = StateInitialize;
  bool m_initialized = false;
  struct MozillaVPNPrivate* m_private = nullptr;

  QTimer m_periodicOperationsTimer;

  bool m_startMinimized = false;
  bool m_updating = false;
  bool m_controllerInitialized = false;
  bool m_locationInitialized = false;
  bool m_isLoggingIn = false;
};

#endif  // MOZILLAVPN_H
