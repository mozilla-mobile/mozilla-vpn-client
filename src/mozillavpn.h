/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLAVPN_H
#define MOZILLAVPN_H

#include "captiveportal/captiveportal.h"
#include "captiveportal/captiveportaldetection.h"
#include "closeeventhandler.h"
#include "connectiondataholder.h"
#include "connectionhealth.h"
#include "constants.h"
#include "controller.h"
#include "errorhandler.h"
#include "models/devicemodel.h"
#include "models/feedbackcategorymodel.h"
#include "models/helpmodel.h"
#include "models/keys.h"
#include "models/licensemodel.h"
#include "models/servercountrymodel.h"
#include "models/serverdata.h"
#include "models/supportcategorymodel.h"
#include "models/surveymodel.h"
#include "models/user.h"
#include "models/whatsnewmodel.h"
#include "networkwatcher.h"
#include "releasemonitor.h"
#include "statusicon.h"
#include "theme.h"

#include <QList>
#include <QNetworkReply>
#include <QObject>
#include <QStandardPaths>
#include <QTimer>

#ifdef MVPN_WINDOWS
#  include "platforms/windows/windowscommons.h"
#endif

class QTextStream;

class MozillaVPN final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(MozillaVPN)

 public:
  enum State {
    StateAuthenticating,
    StateBackendFailure,
    StateBillingNotAvailable,
    StateDeviceLimit,
    StateInitialize,
    StateMain,
    StatePostAuthentication,
    StateSubscriptionBlocked,
    StateSubscriptionNeeded,
    StateSubscriptionInProgress,
    StateSubscriptionNotValidated,
    StateTelemetryPolicy,
    StateUpdateRequired,
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

  enum AlertType {
    NoAlert,
    AuthenticationFailedAlert,
    ConnectionFailedAlert,
    LogoutAlert,
    NoConnectionAlert,
    ControllerErrorAlert,
    RemoteServiceErrorAlert,
    SubscriptionFailureAlert,
    GeoIpRestrictionAlert,
    UnrecoverableErrorAlert,
  };
  Q_ENUM(AlertType)

  enum LinkType {
    LinkAccount,
    LinkContact,
    LinkFeedback,
    LinkLeaveReview,
    LinkHelpSupport,
    LinkTermsOfService,
    LinkPrivacyNotice,
    LinkUpdate,
    LinkInspector,
    LinkSubscriptionBlocked,
    LinkSplitTunnelHelp,
    LinkCaptivePortal
  };
  Q_ENUM(LinkType)

 private:
  Q_PROPERTY(State state READ state NOTIFY stateChanged)
  Q_PROPERTY(AlertType alert READ alert NOTIFY alertChanged)
  Q_PROPERTY(QString versionString READ versionString CONSTANT)
  Q_PROPERTY(QString buildNumber READ buildNumber CONSTANT)
  Q_PROPERTY(QString osVersion READ osVersion CONSTANT)
  Q_PROPERTY(QString architecture READ architecture CONSTANT)
  Q_PROPERTY(QString platform READ platform CONSTANT)
  Q_PROPERTY(bool updateRecommended READ updateRecommended NOTIFY
                 updateRecommendedChanged)
  Q_PROPERTY(UserState userState READ userState NOTIFY userStateChanged)
  Q_PROPERTY(bool startMinimized READ startMinimized CONSTANT)
  Q_PROPERTY(bool updating READ updating NOTIFY updatingChanged)
  Q_PROPERTY(bool stagingMode READ stagingMode CONSTANT)
  Q_PROPERTY(bool debugMode READ debugMode CONSTANT)
  Q_PROPERTY(QString currentView READ currentView WRITE setCurrentView NOTIFY
                 currentViewChanged)

 public:
  MozillaVPN();
  ~MozillaVPN();

  static MozillaVPN* instance();

  // This is exactly like the ::instance() method, but it doesn't crash if the
  // MozillaVPN is null. It should be used rarely.
  static MozillaVPN* maybeInstance();

  void initialize();

  State state() const;
  AlertType alert() const { return m_alert; }

  const QString& serverPublicKey() const { return m_serverPublicKey; }

  bool stagingMode() const;
  bool debugMode() const;

  enum AuthenticationType {
    AuthenticationInBrowser,
    AuthenticationInApp,
  };

  // Exposed QML methods:
  Q_INVOKABLE void getStarted();
  Q_INVOKABLE void authenticate();
  Q_INVOKABLE void cancelAuthentication();
  Q_INVOKABLE void openLink(LinkType linkType);
  Q_INVOKABLE void openLinkUrl(const QString& linkUrl);
  Q_INVOKABLE void removeDeviceFromPublicKey(const QString& publicKey);
  Q_INVOKABLE void hideAlert() { setAlert(NoAlert); }
  Q_INVOKABLE void hideUpdateRecommendedAlert() { setUpdateRecommended(false); }
  Q_INVOKABLE void postAuthenticationCompleted();
  Q_INVOKABLE void telemetryPolicyCompleted();
  Q_INVOKABLE void mainWindowLoaded();
  Q_INVOKABLE bool viewLogs();
  Q_INVOKABLE void retrieveLogs();
  Q_INVOKABLE void cleanupLogs();
  Q_INVOKABLE void storeInClipboard(const QString& text);
  Q_INVOKABLE void activate();
  Q_INVOKABLE void deactivate();
  Q_INVOKABLE void refreshDevices();
  Q_INVOKABLE void update();
  Q_INVOKABLE void backendServiceRestore();
  Q_INVOKABLE void triggerHeartbeat();
  Q_INVOKABLE void submitFeedback(const QString& feedbackText,
                                  const qint8 rating, const QString& category);
  Q_INVOKABLE void openAppStoreReviewLink();
  Q_INVOKABLE void createSupportTicket(const QString& email,
                                       const QString& subject,
                                       const QString& issueText,
                                       const QString& category);
  Q_INVOKABLE bool validateUserDNS(const QString& dns) const;
  Q_INVOKABLE void hardResetAndQuit();
#ifdef MVPN_ANDROID
  Q_INVOKABLE void launchPlayStore();
#endif

  void authenticateWithType(AuthenticationType authenticationType);

  // Internal object getters:
  CaptivePortal* captivePortal() { return &m_private->m_captivePortal; }
  CaptivePortalDetection* captivePortalDetection() {
    return &m_private->m_captivePortalDetection;
  }
  CloseEventHandler* closeEventHandler() {
    return &m_private->m_closeEventHandler;
  }
  ConnectionDataHolder* connectionDataHolder() {
    return &m_private->m_connectionDataHolder;
  }
  ConnectionHealth* connectionHealth() {
    return &m_private->m_connectionHealth;
  }
  Controller* controller() { return &m_private->m_controller; }
  ServerData* currentServer() { return &m_private->m_serverData; }
  DeviceModel* deviceModel() { return &m_private->m_deviceModel; }
  FeedbackCategoryModel* feedbackCategoryModel() {
    return &m_private->m_feedbackCategoryModel;
  }
  SupportCategoryModel* supportCategoryModel() {
    return &m_private->m_supportCategoryModel;
  }
  Keys* keys() { return &m_private->m_keys; }
  LicenseModel* licenseModel() { return &m_private->m_licenseModel; }
  HelpModel* helpModel() { return &m_private->m_helpModel; }
  NetworkWatcher* networkWatcher() { return &m_private->m_networkWatcher; }
  ReleaseMonitor* releaseMonitor() { return &m_private->m_releaseMonitor; }
  ServerCountryModel* serverCountryModel() {
    return &m_private->m_serverCountryModel;
  }
  StatusIcon* statusIcon() { return &m_private->m_statusIcon; }
  SurveyModel* surveyModel() { return &m_private->m_surveyModel; }
  Theme* theme() { return &m_private->m_theme; }
  WhatsNewModel* whatsNewModel() { return &m_private->m_whatsNewModel; }
  User* user() { return &m_private->m_user; }

  // Called at the end of the authentication flow. We can continue adding the
  // device if it doesn't exist yet, or we can go to OFF state.
  void authenticationCompleted(const QByteArray& json, const QString& token);

  void deviceAdded(const QString& deviceName, const QString& publicKey,
                   const QString& privateKey);

  void deviceRemoved(const QString& publicKey);
  void deviceRemovalCompleted(const QString& publicKey);

  void serversFetched(const QByteArray& serverData);

  void accountChecked(const QByteArray& json);

  void surveyChecked(const QByteArray& json);

  const QList<Server> exitServers() const;
  const QList<Server> entryServers() const;
  bool multihop() const { return m_private->m_serverData.multihop(); }

  void errorHandle(ErrorHandler::ErrorType error);

  void abortAuthentication();

  void changeServer(const QString& countryCode, const QString& city,
                    const QString& entryCountryCode = QString(),
                    const QString& entryCity = QString());

  void silentSwitch();

  const QString versionString() const { return QString(APP_VERSION); }
  const QString buildNumber() const { return QString(BUILD_ID); }
  const QString osVersion() const {
#ifdef MVPN_WINDOWS
    return WindowsCommons::WindowsVersion();
#else
    return QSysInfo::productVersion();
#endif
  }
  const QString architecture() const {
    return QSysInfo::currentCpuArchitecture();
  }
  const QString platform() const { return Constants::PLATFORM_NAME; }

  void logout();

  bool updateRecommended() const { return m_updateRecommended; }

  void setUpdateRecommended(bool value);

  UserState userState() const { return m_userState; }

  bool startMinimized() const { return m_startMinimized; }

  void setStartMinimized(bool startMinimized) {
    m_startMinimized = startMinimized;
  }

  void setToken(const QString& token);

  [[nodiscard]] bool setServerList(const QByteArray& serverData);

  Q_INVOKABLE void reset(bool forceInitialState);

  bool modelsInitialized() const;

  void quit();

  bool updating() const { return m_updating; }
  void setUpdating(bool updating);

  void heartbeatCompleted(bool success);

  void setServerPublicKey(const QString& publicKey);

  void addCurrentDeviceAndRefreshData();

  const QString& currentView() const { return m_currentView; }
  void setCurrentView(const QString& name) {
    m_currentView = name;
    emit currentViewChanged();
  }

  void createTicketAnswerRecieved(bool successful) {
    emit ticketCreationAnswer(successful);
  }

  void hardReset();

 private:
  void setState(State state);

  void maybeStateMain();

  void setUserState(UserState userState);

  void startSchedulingPeriodicOperations();

  void stopSchedulingPeriodicOperations();

  void setAlert(AlertType alert);

  bool writeAndShowLogs(QStandardPaths::StandardLocation location);

  bool writeLogs(QStandardPaths::StandardLocation location,
                 std::function<void(const QString& filename)>&& a_callback);

  void serializeLogs(QTextStream* out,
                     std::function<void()>&& finalizeCallback);

  void subscriptionStarted(const QString& productIdentifier);
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

  void maybeRegenerateDeviceKey();

 public slots:
  void requestSettings();
  void requestAbout();
  void requestViewLogs();
  void requestContactUs();

 signals:
  void stateChanged();
  void alertChanged();
  void updateRecommendedChanged();
  void userStateChanged();
  void deviceRemoving(const QString& publicKey);
  void settingsNeeded();
  void aboutNeeded();
  void viewLogsNeeded();
  void contactUsNeeded();
  void updatingChanged();

  // For Glean
  void initializeGlean();
  void sendGleanPings();
  void recordGleanEvent(const QString& gleanSampleName);
  void setGleanSourceTags(const QStringList& tags);

  void aboutToQuit();

  // This is used only on android but, if we use #ifdef MVPN_ANDROID, qml engine
  // complains...
  void loadAndroidAuthenticationView();

  void logsReady(const QString& logs);

  void currentViewChanged();

  void ticketCreationAnswer(bool successful);

 private:
  bool m_initialized = false;

  // Internal objects.
  struct Private {
    CaptivePortal m_captivePortal;
    CaptivePortalDetection m_captivePortalDetection;
    CloseEventHandler m_closeEventHandler;
    ConnectionDataHolder m_connectionDataHolder;
    ConnectionHealth m_connectionHealth;
    Controller m_controller;
    DeviceModel m_deviceModel;
    FeedbackCategoryModel m_feedbackCategoryModel;
    SupportCategoryModel m_supportCategoryModel;
    Keys m_keys;
    LicenseModel m_licenseModel;
    HelpModel m_helpModel;
    NetworkWatcher m_networkWatcher;
    ReleaseMonitor m_releaseMonitor;
    ServerCountryModel m_serverCountryModel;
    ServerData m_serverData;
    StatusIcon m_statusIcon;
    SurveyModel m_surveyModel;
    Theme m_theme;
    WhatsNewModel m_whatsNewModel;
    User m_user;
  };

  Private* m_private = nullptr;

  State m_state = StateInitialize;
  AlertType m_alert = NoAlert;
  QString m_currentView;

  UserState m_userState = UserNotAuthenticated;

  QString m_serverPublicKey;

  QTimer m_alertTimer;
  QTimer m_periodicOperationsTimer;
  QTimer m_gleanTimer;

  bool m_updateRecommended = false;
  bool m_startMinimized = false;
  bool m_updating = false;
  bool m_controllerInitialized = false;
};

#endif  // MOZILLAVPN_H
