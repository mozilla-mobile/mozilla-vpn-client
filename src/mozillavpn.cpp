/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozillavpn.h"

#include "addons/addonapi.h"
#include "addons/manager/addonmanager.h"
#include "authenticationinapp/authenticationinapp.h"
#include "captiveportal/captiveportaldetection.h"
#include "connectionmanager.h"
#include "constants.h"
#include "controller.h"
#include "dnshelper.h"
#include "externalophandler.h"
#include "feature.h"
#include "frontend/navigationbarmodel.h"
#include "frontend/navigator.h"
#include "glean/generated/metrics.h"
#include "glean/generated/pings.h"
#include "glean/mzglean.h"
#include "i18nstrings.h"
#include "inspector/inspectorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"
#include "logoutobserver.h"
#include "models/device.h"
#include "models/devicemodel.h"
#include "models/keys.h"
#include "models/recentconnections.h"
#include "models/recommendedlocationmodel.h"
#include "models/servercountrymodel.h"
#include "mozillavpn_p.h"
#include "networkmanager.h"
#include "networkwatcher.h"
#include "productshandler.h"
#include "profileflow.h"
#include "purchasehandler.h"
#include "qmlengineholder.h"
#include "releasemonitor.h"
#include "serveri18n.h"
#include "settingsholder.h"
#include "settingswatcher.h"
#include "subscriptionmonitor.h"
#include "tasks/account/taskaccount.h"
#include "tasks/adddevice/taskadddevice.h"
#include "tasks/addonindex/taskaddonindex.h"
#include "tasks/authenticate/taskauthenticate.h"
#include "tasks/captiveportallookup/taskcaptiveportallookup.h"
#include "tasks/controlleraction/taskcontrolleraction.h"
#include "tasks/createsupportticket/taskcreatesupportticket.h"
#include "tasks/deleteaccount/taskdeleteaccount.h"
#include "tasks/function/taskfunction.h"
#include "tasks/getfeaturelist/taskgetfeaturelist.h"
#include "tasks/getlocation/taskgetlocation.h"
#include "tasks/getsubscriptiondetails/taskgetsubscriptiondetails.h"
#include "tasks/group/taskgroup.h"
#include "tasks/heartbeat/taskheartbeat.h"
#include "tasks/removedevice/taskremovedevice.h"
#include "tasks/servers/taskservers.h"
#include "taskscheduler.h"
#include "telemetry.h"
#include "tutorialvpn.h"
#include "update/updater.h"
#include "urlopener.h"
#include "versionutils.h"

#ifdef SENTRY_ENABLED
#  include "sentry/sentryadapter.h"
#endif

#ifdef MZ_ANDROID
#  include "platforms/android/androidiaphandler.h"
#  include "platforms/android/androidutils.h"
#  include "platforms/android/androidvpnactivity.h"
#endif

#ifdef MZ_ADJUST
#  include "adjust/adjusthandler.h"
#endif

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QScreen>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

namespace {
Logger logger("MozillaVPN");
MozillaVPN* s_instance = nullptr;
bool s_mockFreeTrial = false;
QString s_updateVersion;
}  // namespace

// static
MozillaVPN* MozillaVPN::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

// static
MozillaVPN* MozillaVPN::maybeInstance() { return s_instance; }

// static
App* App::instance() { return MozillaVPN::instance(); }

MozillaVPN::MozillaVPN() : App(nullptr), m_private(new MozillaVPNPrivate()) {
  MZ_COUNT_CTOR(MozillaVPN);

  logger.debug() << "Creating MozillaVPN singleton";

  Q_ASSERT(!s_instance);
  s_instance = this;

  connect(&m_periodicOperationsTimer, &QTimer::timeout, []() {
    TaskScheduler::scheduleTask(new TaskGroup(
        {new TaskAccount(ErrorHandler::DoNotPropagateError),
         new TaskServers(ErrorHandler::DoNotPropagateError),
         new TaskCaptivePortalLookup(ErrorHandler::DoNotPropagateError),
         new TaskHeartbeat(), new TaskGetFeatureList(), new TaskAddonIndex(),
         new TaskGetSubscriptionDetails(
             TaskGetSubscriptionDetails::NoAuthenticationFlow,
             ErrorHandler::PropagateError)}));
  });

  connect(this, &MozillaVPN::stateChanged, [this]() {
    // If we are activating the app, let's initialize the controller and the
    // periodic tasks.
    // Onboarding requires the connection manager to be initialized so we can
    // request VPN config permissions
    if ((state() == StateMain || state() == StateOnboarding)) {
      // Only initialize if not already initialized
      if (m_private->m_connectionManager.state() ==
          ConnectionManager::State::StateInitializing) {
        m_private->m_connectionManager.initialize();
      }
      startSchedulingPeriodicOperations();
    } else {
      stopSchedulingPeriodicOperations();

      // We don't call deactivate() because that is meant to be used for
      // UI interactions only and it deletes all the pending tasks.
      TaskScheduler::scheduleTask(
          new TaskControllerAction(TaskControllerAction::eDeactivate));
    }
  });

  connect(this, &MozillaVPN::aboutNeeded, this, []() {
    Navigator::instance()->requestScreenFromBottomBar(
        MozillaVPN::ScreenSettings,
        (Navigator::instance()->currentScreen() == MozillaVPN::ScreenSettings
             ? Navigator::ForceReload
             : Navigator::NoFlags));
  });

  connect(&m_private->m_connectionManager, &ConnectionManager::readyToUpdate,
          this, [this]() { setState(StateUpdateRequired); });

  connect(&m_private->m_connectionManager,
          &ConnectionManager::readyToBackendFailure, this, [this]() {
            TaskScheduler::deleteTasks();
            setState(StateBackendFailure);
          });

  connect(&m_private->m_connectionManager,
          &ConnectionManager::readyToServerUnavailable, this,
          [](bool pingReceived) {
            NotificationHandler::instance()->serverUnavailableNotification(
                pingReceived);
          });

  connect(&m_private->m_connectionManager, &ConnectionManager::stateChanged,
          this, &MozillaVPN::controllerStateChanged);

  connect(&m_private->m_connectionManager, &ConnectionManager::stateChanged,
          &m_private->m_statusIcon, &StatusIcon::refreshNeeded);

  connect(this, &MozillaVPN::stateChanged, &m_private->m_statusIcon,
          &StatusIcon::refreshNeeded);

  connect(&m_private->m_connectionHealth, &ConnectionHealth::stabilityChanged,
          &m_private->m_statusIcon, &StatusIcon::refreshNeeded);

  connect(&m_private->m_connectionManager, &ConnectionManager::stateChanged,
          &m_private->m_connectionHealth,
          &ConnectionHealth::connectionStateChanged);

  if (!Feature::get(Feature::Feature_webPurchase)->isSupported()) {
    ProductsHandler::createInstance();
  }
  PurchaseHandler* purchaseHandler = PurchaseHandler::createInstance();
  connect(purchaseHandler, &PurchaseHandler::subscriptionStarted, this,
          &MozillaVPN::subscriptionStarted);
  connect(purchaseHandler, &PurchaseHandler::subscriptionFailed, this,
          &MozillaVPN::subscriptionFailed);
  connect(purchaseHandler, &PurchaseHandler::subscriptionCanceled, this,
          &MozillaVPN::subscriptionCanceled);
  connect(purchaseHandler, &PurchaseHandler::subscriptionCompleted, this,
          &MozillaVPN::subscriptionCompleted);
  connect(purchaseHandler, &PurchaseHandler::restoreSubscriptionStarted, this,
          &MozillaVPN::restoreSubscriptionStarted);
  connect(purchaseHandler, &PurchaseHandler::alreadySubscribed, this,
          &MozillaVPN::alreadySubscribed);
  connect(purchaseHandler, &PurchaseHandler::billingNotAvailable, this,
          &MozillaVPN::billingNotAvailable);
  connect(purchaseHandler, &PurchaseHandler::subscriptionNotValidated, this,
          &MozillaVPN::subscriptionNotValidated);

  registerUrlOpenerLabels();

  registerErrorHandlers();

  registerInspectorCommands();

  registerAddonApis();

  TutorialVPN::registerTutorialSteps();

  registerExternalOperations();

  connect(ErrorHandler::instance(), &ErrorHandler::errorHandled, this,
          &MozillaVPN::errorHandled);

  ensureApplicationIdExists();
}

MozillaVPN::~MozillaVPN() {
  MZ_COUNT_DTOR(MozillaVPN);

  logger.debug() << "Deleting MozillaVPN singleton";

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;

  delete m_private;
}

void MozillaVPN::initialize() {
  logger.debug() << "MozillaVPN Initialization";

  Q_ASSERT(!m_initialized);
  m_initialized = true;

  registerNavigatorScreens();

  registerNavigationBarButtons();

  // This is our first state.
  Q_ASSERT(state() == StateInitialize);

  m_private->m_releaseMonitor.runSoon();

  m_private->m_telemetry.initialize();

  m_private->m_connectionBenchmark.initialize();

  m_private->m_ipAddressLookup.initialize();

  m_private->m_serverLatency.initialize();

  m_private->m_serverData.initialize();

  AddonManager::instance();

  RecentConnections::instance()->initialize();
  RecommendedLocationModel::instance()->initialize();

  QList<Task*> initTasks{new TaskAddonIndex(), new TaskGetFeatureList()};

#ifdef MZ_ADJUST
  initTasks.append(new TaskFunction([] { AdjustHandler::initialize(); }));
#endif

  TaskScheduler::scheduleTask(new TaskGroup(initTasks));

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  SubscriptionMonitor::instance();

#ifdef MZ_ANDROID
  AndroidVPNActivity::maybeInit();
  AndroidUtils::instance();
#endif

  m_private->m_captivePortalDetection.initialize();
  m_private->m_networkWatcher.initialize();

  DNSHelper::maybeMigrateDNSProviderFlags();

  SettingsWatcher::instance();

  if (!settingsHolder->hasToken()) {
    return;
  }

  logger.debug() << "We have a valid token";

  if (!m_private->m_user.fromSettings()) {
    logger.error() << "No user data found";
    return;
  }

  // This step is done to keep users logged in even if they did not complete the
  // subscription. This will fix some of the edge cases for iOS IAP. We do this
  // here as after this point only settings are checked that are set after a
  // successfull subscription.
  if (m_private->m_user.subscriptionNeeded()) {
    setUserState(UserAuthenticated);
    setState(StateAuthenticating);
    TaskScheduler::scheduleTask(
        new TaskFunction([this]() { maybeStateMain(); }));
    return;
  }

  if (!m_private->m_keys.fromSettings()) {
    logger.error() << "No keys found";
    settingsHolder->clear();
    return;
  }

  if (!m_private->m_serverCountryModel.fromSettings()) {
    logger.error() << "No server list found";
    settingsHolder->clear();
    return;
  }

  if (!m_private->m_deviceModel.fromSettings(keys())) {
    logger.error() << "No devices found";
    settingsHolder->clear();
    return;
  }

  if (!checkCurrentDevice()) {
    return;
  }

  if (!m_private->m_captivePortal.fromSettings()) {
    // We do not care about CaptivePortal settings.
  }

  if (!m_private->m_subscriptionData.fromSettings()) {
    // We do not care about SubscriptionData settings.
  }

  if (!modelsInitialized()) {
    logger.error() << "Models not initialized yet";
    settingsHolder->clear();
    return;
  }

  Q_ASSERT(!m_private->m_serverData.hasServerData());
  if (!m_private->m_serverData.fromSettings()) {
    QStringList list = m_private->m_serverCountryModel.pickBest();
    Q_ASSERT(list.length() >= 2);

    m_private->m_serverData.update(list[0], list[1]);
    Q_ASSERT(m_private->m_serverData.hasServerData());
  }

  scheduleRefreshDataTasks();
  setUserState(UserAuthenticated);
  maybeStateMain();
}

void MozillaVPN::maybeStateMain() {
  logger.debug() << "Maybe state main";

  if (m_private->m_user.initialized()) {
    if (state() != StateSubscriptionBlocked &&
        m_private->m_user.subscriptionNeeded()) {
      logger.info() << "Subscription needed";
      setState(StateSubscriptionNeeded);
      return;
    }
    if (state() == StateSubscriptionBlocked) {
      logger.info() << "Subscription is blocked, stay blocked.";
      return;
    }
  }

  SettingsHolder* settingsHolder = SettingsHolder::instance();

  if (!m_private->m_deviceModel.hasCurrentDevice(keys())) {
    Q_ASSERT(m_private->m_deviceModel.activeDevices() ==
             m_private->m_user.maxDevices());
    setState(StateDeviceLimit);
    return;
  }

  // Onboarding needs to come after device limit because on mobile users can
  // choose to add the vpn tunnel configuration and invoke the VPN via system
  // settings after onboarding but before removing a potential 6th device and
  // getting to the home screen
  if (Feature::get(Feature::Feature_newOnboarding)->isSupported()) {
    if (!settingsHolder->onboardingCompleted()) {
      setState(StateOnboarding);
      return;
    }
  }

#if !defined(MZ_ANDROID) && !defined(MZ_IOS)
  if (!settingsHolder->postAuthenticationShown()) {
    setState(StatePostAuthentication);
    return;
  }
#endif

  // If we're not using the new onboarding, continue with the old onboarding
  // (telemetry policy)
  if (!Feature::get(Feature::Feature_newOnboarding)->isSupported()) {
    if (!settingsHolder->telemetryPolicyShown()) {
      setState(StateTelemetryPolicy);
      return;
    }
  }

  if (!modelsInitialized()) {
    logger.warning() << "Models not initialized yet";
    SettingsHolder::instance()->clear();
    REPORTERROR(ErrorHandler::RemoteServiceError, "vpn");

    setUserState(UserNotAuthenticated);
    setState(StateInitialize);
    return;
  }

  Q_ASSERT(m_private->m_serverData.hasServerData());

  // For 2.5 we need to regenerate the device key to allow the the custom DNS
  // feature. We can do it in background when the main view is shown.
  maybeRegenerateDeviceKey();

  if (state() != StateUpdateRequired) {
    // All users who get to StateMain (home screen) should never see onboarding
    // in the future
    settingsHolder->setOnboardingCompleted(true);

    // Resetting for the benefit of testing so that we only have to reset one
    // setting (onboardingCompleted) manually. No real affect on user since they
    // will never see onboarding again
    settingsHolder->setOnboardingStep(0);

    setState(StateMain);
  }

#ifdef MZ_ADJUST
  // When the client is ready to be activated, we do not need adjustSDK anymore
  // (the subscription is done, and no extra events will be dispatched). We
  // cannot disable AdjustSDK at runtime, but we can disable it for the next
  // execution.
  if (settingsHolder->hasAdjustActivatable()) {
    settingsHolder->setAdjustActivatable(false);
  }
#endif
}

void MozillaVPN::authenticate() {
  return authenticateWithType(
      Feature::get(Feature::Feature_inAppAuthentication)->isSupported()
          ? AuthenticationListener::AuthenticationInApp
          : AuthenticationListener::AuthenticationInBrowser);
}

void MozillaVPN::authenticateWithType(
    AuthenticationListener::AuthenticationType authenticationType) {
  logger.debug() << "Authenticate";

  setState(StateAuthenticating);

  ErrorHandler::instance()->hideAlert();

  if (userState() != UserNotAuthenticated) {
    // If we try to start an authentication flow when already logged in, there
    // is a bug elsewhere.
    Q_ASSERT(userState() == UserLoggingOut);

    LogoutObserver* lo = new LogoutObserver(this);
    // Let's use QueuedConnection to avoid nested tasks executions.
    connect(
        lo, &LogoutObserver::ready, this,
        [this, authenticationType]() {
          authenticateWithType(authenticationType);
        },
        Qt::QueuedConnection);
    return;
  }

  TaskScheduler::scheduleTask(new TaskHeartbeat());

  TaskAuthenticate* taskAuthenticate = new TaskAuthenticate(authenticationType);
  connect(taskAuthenticate, &TaskAuthenticate::authenticationAborted, this,
          &MozillaVPN::abortAuthentication);
  connect(taskAuthenticate, &TaskAuthenticate::authenticationCompleted, this,
          &MozillaVPN::completeAuthentication);

  TaskScheduler::scheduleTask(taskAuthenticate);

  emit authenticationStarted();
}

void MozillaVPN::abortAuthentication() {
  logger.warning() << "Abort authentication";
  Q_ASSERT(state() == StateAuthenticating);
  setState(StateInitialize);

  emit authenticationAborted();
}

void MozillaVPN::completeAuthentication(const QByteArray& json,
                                        const QString& token) {
  logger.debug() << "Completing the authentication flow";

  emit authenticationCompleted();

  if (!m_private->m_user.fromJson(json)) {
    logger.error() << "Failed to parse the User JSON data";
    REPORTERROR(ErrorHandler::RemoteServiceError, "vpn");
    return;
  }

  if (!m_private->m_deviceModel.fromJson(keys(), json)) {
    logger.error() << "Failed to parse the DeviceModel JSON data";
    REPORTERROR(ErrorHandler::RemoteServiceError, "vpn");
    return;
  }

  m_private->m_user.writeSettings();
  m_private->m_deviceModel.writeSettings();

  SettingsHolder::instance()->setToken(token);
  setUserState(UserAuthenticated);

  if (m_private->m_user.subscriptionNeeded()) {
    maybeStateMain();
    return;
  }

  if (Feature::get(Feature::Feature_webPurchase)->isSupported()) {
    // Note that if web purchase gets split up so that it is not linked
    // to a fresh authentication task then this can be removed from here
    // as the end of that flow should call subscriptionCompleted.
    PurchaseHandler::instance()->stopSubscription();
  }

  completeActivation();
}

MozillaVPN::RemovalDeviceOption MozillaVPN::maybeRemoveCurrentDevice() {
  logger.debug() << "Maybe remove current device";

  const Device* currentDevice = m_private->m_deviceModel.deviceFromUniqueId();
  if (!currentDevice) {
    logger.debug() << "No removal needed because the device doesn't exist yet";
    return DeviceNotFound;
  }

  if (currentDevice->publicKey() == m_private->m_keys.publicKey() &&
      !m_private->m_keys.privateKey().isEmpty()) {
    logger.debug()
        << "No removal needed because the private key is still fine.";
    return DeviceStillValid;
  }

  logger.debug() << "Removal needed";
  TaskScheduler::scheduleTask(new TaskRemoveDevice(currentDevice->publicKey()));
  return DeviceRemoved;
}

void MozillaVPN::completeActivation() {
  int deviceCount = m_private->m_deviceModel.activeDevices();

  // If we already have a device with the same name, let's remove it.
  RemovalDeviceOption option = maybeRemoveCurrentDevice();
  if (option == DeviceRemoved) {
    --deviceCount;
  }

  if (deviceCount >= m_private->m_user.maxDevices() &&
      option == DeviceNotFound) {
    maybeStateMain();
    return;
  }

  // Here we add the current device.
  if (option != DeviceStillValid) {
    addCurrentDeviceAndRefreshData();
  } else {
    // Let's fetch user and server data.
    scheduleRefreshDataTasks();
  }

  // Finally we are able to activate the client.
  TaskScheduler::scheduleTask(new TaskFunction([this]() { maybeStateMain(); }));
}

void MozillaVPN::setJournalPublicAndPrivateKeys(const QString& publicKey,
                                                const QString& privateKey) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  settingsHolder->setPrivateKeyJournal(privateKey);
  settingsHolder->setPublicKeyJournal(publicKey);
}

void MozillaVPN::resetJournalPublicAndPrivateKeys() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  settingsHolder->setPrivateKeyJournal(QString());
  settingsHolder->setPublicKeyJournal(QString());
}

void MozillaVPN::deviceAdded(const QString& deviceName,
                             const QString& publicKey,
                             const QString& privateKey) {
  Q_UNUSED(publicKey);
  logger.debug() << "Device added" << deviceName;

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  settingsHolder->setPrivateKey(privateKey);
  settingsHolder->setPublicKey(publicKey);
  m_private->m_keys.storeKeys(privateKey, publicKey);

  settingsHolder->setDeviceKeyVersion(Constants::versionString());

  settingsHolder->setKeyRegenerationTimeSec(QDateTime::currentSecsSinceEpoch());
}

void MozillaVPN::removeDevice(const QString& publicKey, const QString& source) {
  logger.debug() << "Remove device";

  m_private->m_deviceModel.removeDeviceFromPublicKey(publicKey);

  emit deviceRemoved(source);

  if (state() != StateDeviceLimit) {
    return;
  }

  // Let's recover from the device-limit mode.
  Q_ASSERT(!m_private->m_deviceModel.hasCurrentDevice(keys()));

  // Here we add the current device.
  addCurrentDeviceAndRefreshData();

  // Finally we are able to activate the client.
  TaskScheduler::scheduleTask(new TaskFunction([this]() {
    if (state() != StateDeviceLimit) {
      return;
    }

    maybeStateMain();
  }));
}

bool MozillaVPN::setServerList(const QByteArray& serverData) {
  if (!m_private->m_serverCountryModel.fromJson(serverData)) {
    logger.error() << "Failed to store the server-countries";
    return false;
  }

  SettingsHolder::instance()->setServers(serverData);
  return true;
}

void MozillaVPN::serversFetched(const QByteArray& serverData) {
  logger.debug() << "Server fetched!";

  if (!setServerList(serverData)) {
    // This is OK. The check is done elsewhere.
    return;
  }

  // The serverData could be unset or invalid with the new server list.
  if (!m_private->m_serverData.hasServerData() ||
      !m_private->m_serverCountryModel.exists(
          m_private->m_serverData.exitCountryCode(),
          m_private->m_serverData.exitCityName())) {
    QStringList list = m_private->m_serverCountryModel.pickBest();
    Q_ASSERT(list.length() >= 2);

    m_private->m_serverData.update(list[0], list[1]);
    Q_ASSERT(m_private->m_serverData.hasServerData());
  }
}

void MozillaVPN::deviceRemovalCompleted(const QString& publicKey) {
  logger.debug() << "Device removal task completed";
  m_private->m_deviceModel.stopDeviceRemovalFromPublicKey(publicKey, keys());
}

void MozillaVPN::removeDeviceFromPublicKey(const QString& publicKey) {
  logger.debug() << "Remove device";

  // Let's emit a signal to inform the user about the starting of the device
  // removal.  The front-end code will show a loading icon or something
  // similar.
  emit deviceRemoving(publicKey);
  TaskScheduler::scheduleTask(new TaskRemoveDevice(publicKey));

  if (state() != StateDeviceLimit) {
    // If we are not in the device-limit state, we can run the operation in
    // background and work aync.
    m_private->m_deviceModel.startDeviceRemovalFromPublicKey(publicKey);
  }
}

void MozillaVPN::createSupportTicket(const QString& email,
                                     const QString& subject,
                                     const QString& issueText,
                                     const QString& category) {
  logger.debug() << "Create support ticket";

  QString* buffer = new QString();
  QTextStream* out = new QTextStream(buffer);

  LogHandler::instance()->serializeLogs(
      out, [out, buffer, email, subject, issueText, category] {
        Q_ASSERT(out);
        Q_ASSERT(buffer);

        // buffer is getting copied by TaskCreateSupportTicket so we can delete
        // it afterwards
        Task* task = new TaskCreateSupportTicket(email, subject, issueText,
                                                 *buffer, category);
        delete buffer;
        delete out;

        // Support tickets can be created at anytime. Even during "critical"
        // operations such as authentication, account deletion, etc. Those
        // operations are often running in tasks, which would block the
        // scheduling of this new support ticket task execution if we used
        // `TaskScheduler::scheduleTask`. To avoid this, let's run this task
        // immediately and let's hope it does not fail.
        TaskScheduler::scheduleTaskNow(task);
      });
}

void MozillaVPN::accountChecked(const QByteArray& json) {
  logger.debug() << "Account checked";

  if (!m_private->m_user.fromJson(json)) {
    logger.warning() << "Failed to parse the User JSON data";
    // We don't need to communicate it to the user. Let's ignore it.
    return;
  }

  if (!m_private->m_deviceModel.fromJson(keys(), json)) {
    logger.warning() << "Failed to parse the DeviceModel JSON data";
    // We don't need to communicate it to the user. Let's ignore it.
    return;
  }

  if (!checkCurrentDevice()) {
    return;
  }

  m_private->m_user.writeSettings();
  m_private->m_deviceModel.writeSettings();

  if (m_private->m_user.subscriptionNeeded() && state() == StateMain) {
    NotificationHandler::instance()->subscriptionNotFoundNotification();
    maybeStateMain();
  }
}

void MozillaVPN::cancelAuthentication() {
  logger.warning() << "Canceling authentication";

  if (state() != StateAuthenticating) {
    // We cannot cancel tasks if we are not in authenticating state.
    return;
  }

  emit authenticationAborted();

  reset(true);
}

bool MozillaVPN::checkCurrentDevice() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  // We are not able to check the device at this stage.
  if (state() == StateDeviceLimit) {
    return false;
  }

  if (m_private->m_deviceModel.hasCurrentDevice(keys())) {
    return true;
  }

  if (settingsHolder->privateKeyJournal().isEmpty() ||
      settingsHolder->publicKeyJournal().isEmpty()) {
    logger.error() << "The current device has not been found";
    settingsHolder->clear();
    return false;
  }

  keys()->storeKeys(settingsHolder->privateKeyJournal(),
                    settingsHolder->publicKeyJournal());
  if (!m_private->m_deviceModel.hasCurrentDevice(keys())) {
    logger.error() << "The current device has not been found (journal keys)";
    settingsHolder->clear();
    return false;
  }

  settingsHolder->setPrivateKey(settingsHolder->privateKey());
  settingsHolder->setPublicKey(settingsHolder->publicKey());
  resetJournalPublicAndPrivateKeys();
  return true;
}

void MozillaVPN::logout() {
  logger.debug() << "Logout";

  ErrorHandler::instance()->requestAlert(ErrorHandler::LogoutAlert);
  setUserState(UserLoggingOut);

  TaskScheduler::deleteTasks();

  PurchaseHandler::instance()->stopSubscription();
  if (!Feature::get(Feature::Feature_webPurchase)->isSupported()) {
    ProductsHandler::instance()->stopProductsRegistration();
  }

  connectionManager()->deleteOSTunnelConfig();

  // update-required state is the only one we want to keep when logging out.
  if (state() != StateUpdateRequired) {
    setState(StateInitialize);
  }

  if (m_private->m_deviceModel.hasCurrentDevice(keys())) {
    TaskScheduler::scheduleTask(new TaskRemoveDevice(keys()->publicKey()));

    // Immediately after the scheduling of the device removal, we want to
    // delete the session token, so that, in case the app is terminated, at
    // the next execution we go back to the init screen.
    reset(false);
    return;
  }

  TaskScheduler::scheduleTask(new TaskFunction([this]() { reset(false); }));
}

void MozillaVPN::reset(bool forceInitialState) {
  logger.debug() << "Cleaning up all";

  deactivate();

  SettingsHolder::instance()->clear();
  m_private->m_keys.forgetKeys();
  m_private->m_serverData.forget();

  connectionManager()->deleteOSTunnelConfig();

  PurchaseHandler::instance()->stopSubscription();
  if (!Feature::get(Feature::Feature_webPurchase)->isSupported()) {
    ProductsHandler::instance()->stopProductsRegistration();
  }

  setUserState(UserNotAuthenticated);

  if (forceInitialState) {
    setState(StateInitialize);
  }
}

void MozillaVPN::postAuthenticationCompleted() {
  logger.debug() << "Post authentication completed";

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  settingsHolder->setPostAuthenticationShown(true);

  // Super racy, but it could happen that we are already in update-required
  // state.
  if (state() == StateUpdateRequired) {
    return;
  }

  maybeStateMain();
}

void MozillaVPN::mainWindowLoaded() {
  logger.debug() << "main window loaded";

  m_private->m_telemetry.stopTimeToFirstScreenTimer();

#ifndef MZ_WASM
  // Initialize glean with an async call because at this time,
  // QQmlEngine does not have root objects yet to see the current
  // graphics API in use.
  logger.debug() << "Initializing Glean";
  QTimer::singleShot(0, this, &MozillaVPN::initializeGlean);

  // Setup regular glean ping sending
  connect(&m_gleanTimer, &QTimer::timeout, this, [this] {
    mozilla::glean_pings::Main.submit();
    emit MozillaVPN::sendGleanPings();
  });
  m_gleanTimer.start(Constants::gleanTimeoutMsec());
  m_gleanTimer.setSingleShot(false);
#endif
#ifdef SENTRY_ENABLED
  SentryAdapter::instance()->init();
  QObject::connect(connectionManager(), &ConnectionManager::readyToQuit,
                   SentryAdapter::instance(), &SentryAdapter::onBeforeShutdown);
#endif
}

void MozillaVPN::onboardingCompleted() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();

  if (Feature::get(Feature::Feature_newOnboarding)->isSupported()) {
    logger.debug() << "onboarding completed";
    settingsHolder->setOnboardingCompleted(true);

    // Toggle glean on or off at the end of onboarding, depending on what the
    // user selected
    settingsHolder->setGleanEnabled(
        settingsHolder->onboardingDataCollectionEnabled());

    // Mark the old onboarding experience as completed as well, ensuring that
    // users do not have to go through it if the new onboaring feature is turned
    // off
    settingsHolder->setPostAuthenticationShown(true);

    // Resetting for the benefit of testing so that we only have to reset one
    // setting (onboardingCompleted) manually. No real affect on user since they
    // *should* never see onboarding more than once
    settingsHolder->setOnboardingStep(0);
    settingsHolder->setOnboardingDataCollectionEnabled(false);

  } else {
    logger.debug() << "telemetry policy completed";
  }

  settingsHolder->setTelemetryPolicyShown(true);

  // Super racy, but it could happen that we are already in update-required
  // state.
  if (state() == StateUpdateRequired) {
    return;
  }

  if (userState() != UserAuthenticated) {
    authenticate();
    return;
  }

  maybeStateMain();
}

void MozillaVPN::startSchedulingPeriodicOperations() {
  logger.debug() << "Start scheduling account and servers"
                 << Constants::schedulePeriodicTaskTimerMsec();
  m_periodicOperationsTimer.start(Constants::schedulePeriodicTaskTimerMsec());
}

void MozillaVPN::stopSchedulingPeriodicOperations() {
  logger.debug() << "Stop scheduling account and servers";
  m_periodicOperationsTimer.stop();
}

bool MozillaVPN::modelsInitialized() const {
  logger.debug() << "Checking model initialization";
  if (!m_private->m_user.initialized()) {
    logger.error() << "User model not initialized";
    return false;
  }

  if (!m_private->m_serverCountryModel.initialized()) {
    logger.error() << "Server country model not initialized";
    return false;
  }

  if (!m_private->m_deviceModel.initialized()) {
    logger.error() << "Device model not initialized";
    return false;
  }

  if (!m_private->m_deviceModel.hasCurrentDevice(&m_private->m_keys)) {
    logger.error() << "Current device not registered";
    return false;
  }

  if (!m_private->m_keys.initialized()) {
    logger.error() << "Key model not initialized";
    return false;
  }

  return true;
}

void MozillaVPN::requestAbout() {
  logger.debug() << "About view requested";

  QmlEngineHolder::instance()->showWindow();
  emit aboutNeeded();
}

void MozillaVPN::activate() {
  logger.debug() << "VPN tunnel activation";

  TaskScheduler::deleteTasks();

  // We are about to connect. If the device key needs to be regenerated, this
  // is the right time to do it.
  maybeRegenerateDeviceKey();

  TaskScheduler::scheduleTask(
      new TaskControllerAction(TaskControllerAction::eActivate));
}

void MozillaVPN::deactivate(bool block) {
  logger.debug() << "VPN tunnel deactivation";

  TaskScheduler::deleteTasks();
  Task* task = new TaskControllerAction(TaskControllerAction::eDeactivate);
  if (block) {
    connect(task, &Task::completed, this, [&]() { block = false; });
  }
  TaskScheduler::scheduleTask(task);

  while (block) {
    QCoreApplication::processEvents();
  }
}

void MozillaVPN::silentSwitch() {
  logger.debug() << "VPN tunnel silent server switch";

  // Let's delete all the tasks before running the silent-switch op. If we are
  // here, the connection does not work and we don't want to wait for timeouts
  // to run the silenct-switch.
  TaskScheduler::deleteTasks();
  TaskScheduler::scheduleTask(
      new TaskControllerAction(TaskControllerAction::eSilentSwitch,
                               ConnectionManager::eServerCoolDownNeeded));
}

void MozillaVPN::refreshDevices() {
  logger.debug() << "Refresh devices";

  if (state() == StateMain) {
    TaskScheduler::scheduleTask(
        new TaskAccount(ErrorHandler::DoNotPropagateError));
  }
}

void MozillaVPN::subscriptionStarted(const QString& productIdentifier) {
  logger.debug() << "Subscription started" << productIdentifier;

  setState(StateSubscriptionInProgress);
  if (!Feature::get(Feature::Feature_webPurchase)->isSupported()) {
    ProductsHandler* products = ProductsHandler::instance();
    Q_ASSERT(products->hasProductsRegistered());
  }

  PurchaseHandler::instance()->startSubscription(productIdentifier);
}

void MozillaVPN::restoreSubscriptionStarted() {
  logger.debug() << "Restore subscription started";
  setState(StateSubscriptionInProgress);
  PurchaseHandler::instance()->startRestoreSubscription();
}

void MozillaVPN::subscriptionCompleted() {
  emit logSubscriptionCompleted();
#ifdef MZ_ANDROID
  // This is Android only
  // iOS can end up here if the subsciption get finished outside of the IAP
  // process
  if (state() != StateSubscriptionInProgress) {
    // We could hit this in android flow if we're doing a late acknowledgement.
    // And ignoring is fine.
    logger.warning()
        << "Random subscription completion received. Let's ignore it.";
    return;
  }
#endif

  logger.debug() << "Subscription completed";

#ifdef MZ_ADJUST
  AdjustHandler::trackEvent(Constants::ADJUST_SUBSCRIPTION_COMPLETED);
#endif

  completeActivation();
}

void MozillaVPN::billingNotAvailable() {
  // If a subscription isn't needed, billingNotAvailable is
  // a no-op because we don't need the billing client.
  if (m_private->m_user.subscriptionNeeded()) {
    setState(StateBillingNotAvailable);
  }
}

void MozillaVPN::subscriptionNotValidated() {
  setState(StateSubscriptionNotValidated);
}

void MozillaVPN::subscriptionFailed() {
  subscriptionFailedInternal(false /* canceled by user */);
}

void MozillaVPN::subscriptionCanceled() {
  subscriptionFailedInternal(true /* canceled by user */);
}

void MozillaVPN::subscriptionFailedInternal(bool canceledByUser) {
#ifdef MZ_IOS
  // This is iOS only.
  // Android can legitimately end up here on a skuDetailsFailed.
  if (state() != StateSubscriptionInProgress) {
    logger.warning()
        << "Random subscription failure received. Let's ignore it.";
    return;
  }
#endif

  logger.debug() << "Subscription failed or canceled";

  // Let's go back to the subscription needed.
  setState(StateSubscriptionNeeded);

  if (!canceledByUser) {
    REPORTERROR(ErrorHandler::SubscriptionFailureError, "vpn");
  }

  TaskScheduler::scheduleTask(new TaskFunction([this]() {
    if (!m_private->m_user.subscriptionNeeded() &&
        state() == StateSubscriptionNeeded) {
      maybeStateMain();
      return;
    }
  }));
}

void MozillaVPN::alreadySubscribed() {
#ifdef MZ_IOS
  // This randomness is an iOS only issue
  // TODO - How can we make this cleaner in the future
  if (state() != StateSubscriptionInProgress) {
    logger.warning()
        << "Random already-subscribed notification received. Let's ignore it.";
    return;
  }
#endif

  logger.info() << "Setting state: Subscription Blocked";
  setState(StateSubscriptionBlocked);
}

void MozillaVPN::update() {
  logger.debug() << "Update";

  setUpdating(true);

  // The windows installer will stop the client and daemon before installation
  // so it's not necessary to disable the VPN to perform an upgrade.
#ifndef MZ_WINDOWS
  if (m_private->m_connectionManager.state() != ConnectionManager::StateOff &&
      m_private->m_connectionManager.state() !=
          ConnectionManager::StateInitializing) {
    deactivate();
    return;
  }
#endif

  m_private->m_releaseMonitor.updateSoon();
}

void MozillaVPN::setUpdating(bool updating) {
  m_updating = updating;
  emit updatingChanged();
}

void MozillaVPN::controllerStateChanged() {
  logger.debug() << "Controller state changed";

  if (!m_controllerInitialized) {
    m_controllerInitialized = true;

    if (SettingsHolder::instance()->startAtBoot()) {
      logger.debug() << "Start on boot";
      activate();
    }
  }

  if (m_updating &&
      m_private->m_connectionManager.state() == ConnectionManager::StateOff) {
    update();
  }

  NetworkManager::instance()->clearCache();
}

void MozillaVPN::backendServiceRestore() {
  logger.debug() << "Background service restore request";
  // TODO
}

void MozillaVPN::heartbeatCompleted(bool success) {
  logger.debug() << "Server-side check done:" << success;

  // In the event of a Guardian error during authentication,
  // the "something went wrong" screen is displayed to the user.
  // This is important because the service used for authentication is
  // unavailable and causes an interruption in the authentication process.
  // Alternatively if the user is already authenticated, we intentionally avoid
  // presenting them with the "something went wrong" screen to avoid unnecessary
  // interruptions. If the user is already authenticated, it is possible that a
  // Guardian service may momentarily become unavailable but there is nothing
  // for the user to do and it may not affect the connectivity at all so it is
  // not necessary to take additional actions.
  if (!success && state() == StateAuthenticating) {
    m_private->m_connectionManager.backendFailure();
    return;
  }

  if (state() != StateBackendFailure) {
    return;
  }

  if (!modelsInitialized() || userState() != UserAuthenticated) {
    setState(StateInitialize);
    return;
  }

  maybeStateMain();
}

void MozillaVPN::triggerHeartbeat() {
  TaskScheduler::scheduleTask(new TaskHeartbeat());
}

void MozillaVPN::addCurrentDeviceAndRefreshData() {
  TaskScheduler::scheduleTask(
      new TaskAddDevice(Device::currentDeviceName(), Device::uniqueDeviceId()));
  scheduleRefreshDataTasks();
}

bool MozillaVPN::validateUserDNS(const QString& dns) const {
  return DNSHelper::validateUserDNS(dns);
}

void MozillaVPN::maybeRegenerateDeviceKey() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  if (settingsHolder->hasDeviceKeyVersion() &&
      VersionUtils::compareVersions(settingsHolder->deviceKeyVersion(),
                                    "2.5.0") >= 0) {
    return;
  }

  // We need a new device key only if the user wants to use custom DNS servers.
  if (settingsHolder->dnsProviderFlags() ==
      SettingsHolder::DNSProviderFlags::Gateway) {
    logger.debug() << "Removal needed but no custom DNS used.";
    return;
  }

  Q_ASSERT(m_private->m_deviceModel.hasCurrentDevice(keys()));

  logger.debug() << "Removal needed for the 2.5 key regeneration.";

  // We do not need to remove the current device! guardian-website "overwrites"
  // the current device key when we submit a new one.
  addCurrentDeviceAndRefreshData();
  TaskScheduler::scheduleTask(new TaskFunction([this]() {
    if (!modelsInitialized()) {
      logger.error() << "Failed to complete the key regeneration";
      REPORTERROR(ErrorHandler::RemoteServiceError, "vpn");
      setUserState(UserNotAuthenticated);
      return;
    }
  }));
}

void MozillaVPN::hardReset() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);
  settingsHolder->hardReset();
  connectionManager()->deleteOSTunnelConfig();
}

void MozillaVPN::hardResetAndQuit() {
  logger.debug() << "Hard reset and quit";
  hardReset();
  // Deactivate VPN and quit
  connectionManager()->quit();
}

void MozillaVPN::requestDeleteAccount() {
  logger.debug() << "delete account";
  Q_ASSERT(Feature::get(Feature::Feature_accountDeletion)->isSupported());

  TaskDeleteAccount* task = new TaskDeleteAccount(m_private->m_user.email());
  connect(task, &TaskDeleteAccount::accountDeleted, this,
          &MozillaVPN::accountDeleted);

  TaskScheduler::scheduleTask(task);
}

void MozillaVPN::cancelReauthentication() {
  logger.warning() << "Canceling reauthentication";
  AuthenticationInApp::instance()->terminateSession();

  cancelAuthentication();
}

void MozillaVPN::updateViewShown() {
  logger.debug() << "Update view shown";
  Updater::updateViewShown();
}

void MozillaVPN::scheduleRefreshDataTasks() {
  QList<Task*> refreshTasks{
      new TaskAccount(ErrorHandler::PropagateError),
      new TaskServers(ErrorHandler::PropagateError),
      new TaskCaptivePortalLookup(ErrorHandler::PropagateError),
      new TaskGetSubscriptionDetails(
          TaskGetSubscriptionDetails::NoAuthenticationFlow,
          ErrorHandler::PropagateError)};

  // The VPN needs to be off in order to determine the client's real location.
  // And it also needs to complete before TaskServers in case this triggers an
  // automatic server selection.
  //
  // TODO: This ordering requirement can be relaxed in the future once automatic
  // server selection is implemented upon activation. See JIRA issue
  // https://mozilla-hub.atlassian.net/browse/VPN-3726 for more information.
  if (!m_private->m_location.initialized()) {
    ConnectionManager::State st = m_private->m_connectionManager.state();
    if (st == ConnectionManager::StateOff ||
        st == ConnectionManager::StateInitializing) {
      TaskScheduler::scheduleTask(
          new TaskGetLocation(ErrorHandler::PropagateError));
    }
  }

  TaskScheduler::scheduleTask(new TaskGroup(refreshTasks));
}

// static
void MozillaVPN::registerUrlOpenerLabels() {
  UrlOpener* uo = UrlOpener::instance();

  uo->registerUrlLabel("captivePortal", []() -> QString {
    SettingsHolder* settingsHolder = SettingsHolder::instance();

    return Constants::captivePortalUrl().arg(
        settingsHolder->captivePortalIpv4Addresses().isEmpty()
            ? "127.0.0.1"
            : settingsHolder->captivePortalIpv4Addresses().first());
  });

  uo->registerUrlLabel("inspector", []() -> QString {
    return "https://mozilla-mobile.github.io/mozilla-vpn-client/inspector/";
  });

  uo->registerUrlLabel("privacyNotice", []() -> QString {
    return Constants::apiUrl(Constants::RedirectPrivacy);
  });

  // TODO: This should link to a more helpful article
  uo->registerUrlLabel("splitTunnelHelp", []() -> QString {
    return "https://support.mozilla.org/kb/"
           "split-tunneling-use-mozilla-vpn-specific-apps-wind";
  });

  uo->registerUrlLabel("subscriptionBlocked", []() -> QString {
    return Constants::apiUrl(Constants::RedirectSubscriptionBlocked);
  });

  uo->registerUrlLabel("subscriptionIapApple", []() -> QString {
    return Constants::APPLE_SUBSCRIPTIONS_URL;
  });

  uo->registerUrlLabel("subscriptionIapGoogle", []() -> QString {
    return Constants::GOOGLE_SUBSCRIPTIONS_URL;
  });

  uo->registerUrlLabel("subscriptionFxa", []() -> QString {
    return QString("%1/subscriptions").arg(Constants::fxaUrl());
  });

  uo->registerUrlLabel("contactSupport", []() -> QString {
    return Constants::contactSupportUrl();
  });

  uo->registerUrlLabel(
      "sumo", []() -> QString { return Constants::MOZILLA_VPN_SUMO_URL; });

  uo->registerUrlLabel("termsOfService", []() -> QString {
    return Constants::apiUrl(Constants::RedirectTermsOfService);
  });

  uo->registerUrlLabel("update", []() -> QString {
    return
#if defined(MZ_IOS)
        Constants::APPLE_STORE_URL
#elif defined(MZ_ANDROID)
                              Constants::GOOGLE_PLAYSTORE_URL
#else
            Constants::apiUrl(
                Constants::RedirectUpdateWithPlatformArgument)
                .arg(Constants::PLATFORM_NAME)
#endif
        ;
  });
  uo->registerUrlLabel("upgradeToAnnualUrl", []() -> QString {
    return Constants::upgradeToAnnualUrl();
  });
}

void MozillaVPN::errorHandled() {
  ErrorHandler::AlertType alert = ErrorHandler::instance()->alert();

  // Any error in authenticating state sends to the Initial state.
  if (state() == App::StateAuthenticating) {
    if (alert == ErrorHandler::GeoIpRestrictionAlert) {
      mozilla::glean::sample::authentication_failure_by_geo.record();
    } else {
      mozilla::glean::sample::authentication_failure.record();
    }

    reset(true);
    return;
  }

  if (alert == ErrorHandler::AuthenticationFailedAlert) {
    reset(true);
  }
}

void MozillaVPN::registerErrorHandlers() {
  ErrorHandler::registerCustomErrorHandler(
      ErrorHandler::NoConnectionError, true, []() {
        MozillaVPN* vpn = MozillaVPN::instance();
        return vpn->connectionHealth() && vpn->connectionHealth()->isUnsettled()
                   ? ErrorHandler::NoAlert
                   : ErrorHandler::NoConnectionAlert;
      });

  ErrorHandler::registerCustomErrorHandler(
      ErrorHandler::DependentConnectionError, true, []() {
        MozillaVPN* vpn = MozillaVPN::instance();
        if (vpn->connectionManager()->state() ==
                ConnectionManager::State::StateOn ||
            vpn->connectionManager()->state() ==
                ConnectionManager::State::StateConfirming) {
          // connection likely isn't stable yet
          logger.error()
              << "Ignore network error probably caused by enabled VPN";
          return ErrorHandler::NoAlert;
        }

        if (vpn->connectionManager()->state() ==
            ConnectionManager::State::StateOff) {
          // We are off, so this means a request failed, not the
          // VPN. Change it to No Connection
          return ErrorHandler::NoConnectionAlert;
        }

        return ErrorHandler::ConnectionFailedAlert;
      });
}

void MozillaVPN::registerNavigatorScreens() {
  Navigator::registerScreen(
      MozillaVPN::ScreenAuthenticating, Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenAuthenticating.qml",
      QVector<int>{App::StateAuthenticating},
      [](int*) -> int8_t {
        return Feature::get(Feature::Feature_inAppAuthentication)->isSupported()
                   ? -1
                   : 0;
      },
      []() -> bool {
        MozillaVPN::instance()->cancelAuthentication();
        return true;
      });

  Navigator::registerScreen(
      MozillaVPN::ScreenAuthenticationInApp,
      Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenAuthenticationInApp.qml",
      QVector<int>{App::StateAuthenticating},
      [](int*) -> int8_t {
        return Feature::get(Feature::Feature_inAppAuthentication)->isSupported()
                   ? 0
                   : -1;
      },
      []() -> bool {
        MozillaVPN::instance()->cancelAuthentication();
        return true;
      });

  Navigator::registerScreen(
      MozillaVPN::ScreenBackendFailure, Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenBackendFailure.qml",
      QVector<int>{MozillaVPN::StateBackendFailure},
      [](int*) -> int8_t { return 0; }, []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenBillingNotAvailable,
      Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenBillingNotAvailable.qml",
      QVector<int>{App::StateBillingNotAvailable},
      [](int*) -> int8_t { return 0; }, []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenCaptivePortal, Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenCaptivePortal.qml", QVector<int>{App::StateMain},
      [](int*) -> int8_t { return 0; }, []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenCrashReporting, Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenCrashReporting.qml", QVector<int>{},
      [](int* requestedScreen) -> int8_t {
        return (requestedScreen &&
                *requestedScreen == MozillaVPN::ScreenCrashReporting)
                   ? 99
                   : -1;
      },
      []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenDeleteAccount, Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenDeleteAccount.qml", QVector<int>{App::StateMain},
      [](int* requestedScreen) -> int8_t {
        return (requestedScreen &&
                *requestedScreen == MozillaVPN::ScreenDeleteAccount)
                   ? 99
                   : -1;
      },
      []() -> bool {
        MozillaVPN::instance()->cancelReauthentication();
        return false;
      });

  Navigator::registerScreen(
      MozillaVPN::ScreenDeviceLimit, Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenDeviceLimit.qml",
      QVector<int>{MozillaVPN::StateDeviceLimit},
      [](int*) -> int8_t { return 0; }, []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenGetHelp, Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenGetHelp.qml", QVector<int>{},
      [](int* requestedScreen) -> int8_t {
        return (requestedScreen &&
                *requestedScreen == MozillaVPN::ScreenGetHelp)
                   ? 99
                   : -1;
      },
      []() -> bool {
        Navigator::instance()->requestPreviousScreen();
        return true;
      });

  Navigator::registerScreen(
      MozillaVPN::ScreenHome, Navigator::LoadPolicy::LoadPersistently,
      "qrc:/ui/screens/ScreenHome.qml", QVector<int>{App::StateMain},
      [](int*) -> int8_t { return 99; }, []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenInitialize, Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenInitialize.qml",
      QVector<int>{App::StateInitialize}, [](int*) -> int8_t { return 0; },
      []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenMessaging, Navigator::LoadPolicy::LoadPersistently,
      "qrc:/ui/screens/ScreenMessaging.qml", QVector<int>{App::StateMain},
      [](int*) -> int8_t { return 0; },
      []() -> bool {
        Navigator::instance()->requestScreen(MozillaVPN::ScreenHome,
                                             Navigator::ForceReload);
        return true;
      });

  Navigator::registerScreen(
      MozillaVPN::ScreenNoSubscriptionFoundError,
      Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenNoSubscriptionFoundError.qml",
      QVector<int>{App::StateSubscriptionNeeded,
                   App::StateSubscriptionInProgress},
      [](int* requestedScreen) -> int8_t {
        return (requestedScreen &&
                *requestedScreen == MozillaVPN::ScreenNoSubscriptionFoundError)
                   ? 99
                   : -1;
      },
      []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenPostAuthentication,
      Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenPostAuthentication.qml",
      QVector<int>{App::StatePostAuthentication},
      [](int*) -> int8_t { return 0; }, []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenSettings, Navigator::LoadPolicy::LoadPersistently,
      "qrc:/ui/screens/ScreenSettings.qml", QVector<int>{App::StateMain},
      [](int*) -> int8_t { return 0; },
      []() -> bool {
        Navigator::instance()->requestScreen(MozillaVPN::ScreenHome,
                                             Navigator::ForceReload);
        return true;
      });

  Navigator::registerScreen(
      MozillaVPN::ScreenSubscriptionBlocked,
      Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenSubscriptionBlocked.qml",
      QVector<int>{App::StateSubscriptionBlocked},
      [](int*) -> int8_t { return 0; }, []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenSubscriptionExpiredError,
      Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenSubscriptionExpiredError.qml",
      QVector<int>{App::StateSubscriptionNeeded,
                   App::StateSubscriptionInProgress},
      [](int* requestedScreen) -> int8_t {
        return requestedScreen && *requestedScreen ==
                                      MozillaVPN::ScreenSubscriptionExpiredError
                   ? 99
                   : -1;
      },
      []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenSubscriptionGenericError,
      Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenSubscriptionGenericError.qml",
      QVector<int>{App::StateSubscriptionNeeded,
                   App::StateSubscriptionInProgress},
      [](int* requestedScreen) -> int8_t {
        return (requestedScreen &&
                *requestedScreen == MozillaVPN::ScreenSubscriptionGenericError)
                   ? 99
                   : -1;
      },
      []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenSubscriptionInProgressIAP,
      Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenSubscriptionInProgressIAP.qml",
      QVector<int>{App::StateSubscriptionInProgress},
      [](int*) -> int8_t {
        return Feature::get(Feature::Feature_webPurchase)->isSupported() ? -1
                                                                         : 99;
      },
      []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenSubscriptionInProgressWeb,
      Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenSubscriptionInProgressWeb.qml",
      QVector<int>{App::StateSubscriptionInProgress},
      [](int*) -> int8_t {
        return Feature::get(Feature::Feature_webPurchase)->isSupported() ? 99
                                                                         : -1;
      },
      []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenSubscriptionInUseError,
      Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenSubscriptionInUseError.qml",
      QVector<int>{App::StateSubscriptionNeeded,
                   App::StateSubscriptionInProgress},
      [](int* requestedScreen) -> int8_t {
        return requestedScreen && *requestedScreen ==
                                      MozillaVPN::ScreenSubscriptionInUseError
                   ? 99
                   : -1;
      },
      []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenSubscriptionNeeded,
      Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenSubscriptionNeeded.qml",
      QVector<int>{App::StateSubscriptionNeeded},
      [](int*) -> int8_t { return 0; }, []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenSubscriptionNotValidated,
      Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenSubscriptionNotValidated.qml",
      QVector<int>{App::StateSubscriptionNotValidated},
      [](int*) -> int8_t { return 0; }, []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenTelemetryPolicy, Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenTelemetryPolicy.qml",
      QVector<int>{App::StateTelemetryPolicy}, [](int*) -> int8_t { return 0; },
      []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenUpdateRequired, Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenUpdateRequired.qml",
      QVector<int>{MozillaVPN::StateUpdateRequired},
      [](int*) -> int8_t { return 0; }, []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenUpdateRecommended,
      Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenUpdateRecommended.qml", QVector<int>{},
      [](int* requestedScreen) -> int8_t {
        return (requestedScreen &&
                *requestedScreen == MozillaVPN::ScreenUpdateRecommended)
                   ? 99
                   : -1;
      },
      []() -> bool {
        Navigator::instance()->requestPreviousScreen();
        return true;
      });

  Navigator::registerScreen(
      MozillaVPN::ScreenTipsAndTricks, Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenTipsAndTricks.qml", QVector<int>{},
      [](int* requestedScreen) -> int8_t {
        return (requestedScreen &&
                *requestedScreen == MozillaVPN::ScreenTipsAndTricks)
                   ? 99
                   : -1;
      },
      []() -> bool {
        Navigator::instance()->requestPreviousScreen();
        return true;
      });

  Navigator::registerScreen(
      MozillaVPN::ScreenViewLogs, Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenViewLogs.qml", QVector<int>{},
      [](int* requestedScreen) -> int8_t {
        return (requestedScreen &&
                *requestedScreen == MozillaVPN::ScreenViewLogs)
                   ? 99
                   : -1;
      },
      []() -> bool {
        Navigator::instance()->requestPreviousScreen();
        return true;
      });

  Navigator::registerScreen(
      MozillaVPN::ScreenRemovingDevice, Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenRemovingDevice.qml", QVector<int>{},
      [](int* requestedScreen) -> int8_t {
        return (requestedScreen &&
                *requestedScreen == MozillaVPN::ScreenRemovingDevice)
                   ? 99
                   : -1;
      },
      []() -> bool {
        Navigator::instance()->requestPreviousScreen();
        return true;
      });

  Navigator::registerScreen(
      MozillaVPN::ScreenOnboarding, Navigator::LoadPolicy::LoadTemporarily,
      "qrc:/ui/screens/ScreenOnboarding.qml",
      QVector<int>{App::StateOnboarding}, [](int*) -> int8_t { return 0; },
      []() -> bool { return false; });

  connect(ErrorHandler::instance(), &ErrorHandler::noSubscriptionFound,
          Navigator::instance(), []() {
            Navigator::instance()->requestScreen(
                MozillaVPN::ScreenNoSubscriptionFoundError);
          });

  connect(ErrorHandler::instance(), &ErrorHandler::subscriptionExpired,
          Navigator::instance(), []() {
            Navigator::instance()->requestScreen(
                MozillaVPN::ScreenSubscriptionExpiredError);
          });

  connect(ErrorHandler::instance(), &ErrorHandler::subscriptionInUse,
          Navigator::instance(), []() {
            Navigator::instance()->requestScreen(
                MozillaVPN::ScreenSubscriptionInUseError);
          });

  connect(
      ErrorHandler::instance(), &ErrorHandler::subscriptionGeneric,
      Navigator::instance(), []() {
        Navigator::instance()->requestScreen(ScreenSubscriptionGenericError);
      });

#ifdef SENTRY_ENABLED
  connect(
      SentryAdapter::instance(), &SentryAdapter::needsCrashReportScreen,
      Navigator::instance(), []() {
        Navigator::instance()->requestScreen(MozillaVPN::ScreenCrashReporting);
      });
#endif

  Navigator::instance()->initialize();
}

static void resetNotification(NavigationBarButton* icon) {
  int unreadMessages = 0;
  AddonManager::instance()->forEach(
      [unreadMessages = &unreadMessages](Addon* addon) {
        if (addon->type() == "message" && !addon->property("isRead").toBool()) {
          (*unreadMessages)++;
        }
      });

  icon->setHasNotification(!!unreadMessages);
}

// static
void MozillaVPN::registerNavigationBarButtons() {
  NavigationBarModel* nbm = NavigationBarModel::instance();
  nbm->appendButton(new NavigationBarButton(
      nbm, "navButton-home", "NavBarHomeTab", MozillaVPN::ScreenHome,
      "qrc:/nebula/resources/navbar/home.svg",
      "qrc:/nebula/resources/navbar/home-selected.svg"));

  NavigationBarButton* messageIcon = new NavigationBarButton(
      nbm, "navButton-messages", "NavBarMessagesTab",
      MozillaVPN::ScreenMessaging, "qrc:/nebula/resources/navbar/messages.svg",
      "qrc:/nebula/resources/navbar/messages-selected.svg",
      "qrc:/nebula/resources/navbar/messages-notification.svg",
      "qrc:/nebula/resources/navbar/messages-notification-selected.svg");
  nbm->appendButton(messageIcon);

  nbm->appendButton(new NavigationBarButton(
      nbm, "navButton-settings", "NavBarSettingsTab",
      MozillaVPN::ScreenSettings, "qrc:/nebula/resources/navbar/settings.svg",
      "qrc:/nebula/resources/navbar/settings-selected.svg"));

  connect(SettingsHolder::instance(), &SettingsHolder::addonSettingsChanged,
          [messageIcon]() { resetNotification(messageIcon); });

  connect(AddonManager::instance(), &AddonManager::loadCompletedChanged,
          [messageIcon]() { resetNotification(messageIcon); });

  resetNotification(messageIcon);
}

// static
bool MozillaVPN::mockFreeTrial() { return s_mockFreeTrial; }

// static
void MozillaVPN::registerInspectorCommands() {
  InspectorHandler::setConstructorCallback(
      [](InspectorHandler* inspectorHandler) {
        connect(
            NotificationHandler::instance(),
            &NotificationHandler::notificationShown, inspectorHandler,
            [inspectorHandler](const QString& title, const QString& message) {
              QJsonObject obj;
              obj["type"] = "notification";
              obj["title"] = title;
              obj["message"] = message;
              inspectorHandler->send(
                  QJsonDocument(obj).toJson(QJsonDocument::Compact));
            });

        connect(AddonManager::instance(), &AddonManager::loadCompletedChanged,
                inspectorHandler, [inspectorHandler]() {
                  QJsonObject obj;
                  obj["type"] = "addon_load_completed";
                  inspectorHandler->send(
                      QJsonDocument(obj).toJson(QJsonDocument::Compact));
                });
      });

#ifdef MZ_ANDROID
  InspectorHandler::registerCommand(
      "android_daemon", "Send a request to the Daemon {type} {args}", 2,
      [](InspectorHandler*, const QList<QByteArray>& arguments) {
        auto activity = AndroidVPNActivity::instance();
        Q_ASSERT(activity);
        auto type = QString(arguments[1]);
        auto json = QString(arguments[2]);

        ServiceAction a = (ServiceAction)type.toInt();
        AndroidVPNActivity::sendToService(a, json);
        return QJsonObject();
      });
#endif

  InspectorHandler::registerCommand(
      "activate", "Activate the VPN", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        MozillaVPN::instance()->activate();
        return QJsonObject();
      });

  InspectorHandler::registerCommand(
      "click_notification", "Click on a notification", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        NotificationHandler::instance()->messageClickHandle();
        return QJsonObject();
      });

  InspectorHandler::registerCommand(
      "deactivate", "Deactivate the VPN", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        MozillaVPN::instance()->deactivate();
        return QJsonObject();
      });

  InspectorHandler::registerCommand(
      "devices", "Retrieve the list of devices", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        MozillaVPN* vpn = MozillaVPN::instance();
        Q_ASSERT(vpn);

        DeviceModel* dm = vpn->deviceModel();
        Q_ASSERT(dm);

        QJsonArray deviceArray;
        for (const Device& device : dm->devices()) {
          QJsonObject deviceObj;
          deviceObj["name"] = device.name();
          deviceObj["publicKey"] = device.publicKey();
          deviceObj["currentDevice"] = device.isCurrentDevice(vpn->keys());
          deviceArray.append(deviceObj);
        }

        QJsonObject obj;
        obj["value"] = deviceArray;
        return obj;
      });

  InspectorHandler::registerCommand(
      "guides", "Returns a list of guide title ids", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        QJsonObject obj;

        AddonManager* am = AddonManager::instance();
        Q_ASSERT(am);

        QJsonArray guides;
        am->forEach([&](Addon* addon) {
          if (addon->type() == "guide") {
            guides.append(addon->id());
          }
        });

        obj["value"] = guides;
        return obj;
      });

  InspectorHandler::registerCommand(
      "fetch_addons", "Force a fetch of the addon list manifest", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        AddonManager::instance()->fetch();
        return QJsonObject();
      });

  InspectorHandler::registerCommand(
      "force_captive_portal_check", "Force a captive portal check", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        MozillaVPN::instance()->captivePortalDetection()->detectCaptivePortal();
        return QJsonObject();
      });

  InspectorHandler::registerCommand(
      "force_captive_portal_detection", "Simulate a captive portal detection",
      0, [](InspectorHandler*, const QList<QByteArray>&) {
        MozillaVPN::instance()
            ->captivePortalDetection()
            ->captivePortalDetected();
        MozillaVPN::instance()->connectionManager()->captivePortalPresent();
        return QJsonObject();
      });

  InspectorHandler::registerCommand(
      "force_heartbeat_failure", "Force a heartbeat failure", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        MozillaVPN::instance()->heartbeatCompleted(false /* success */);
        return QJsonObject();
      });

  InspectorHandler::registerCommand(
      "force_server_unavailable",
      "Timeout all servers in a city using force_server_unavailable "
      "{countryCode} "
      "{cityCode}",
      2, [](InspectorHandler*, const QList<QByteArray>& arguments) {
        QJsonObject obj;
        if (QString(arguments[1]) != "" && QString(arguments[2]) != "") {
          MozillaVPN::instance()
              ->serverCountryModel()
              ->setCooldownForAllServersInACity(QString(arguments[1]),
                                                QString(arguments[2]));
        } else {
          obj["error"] =
              QString("Please provide country and city codes as arguments");
        }

        return QJsonObject();
      });

  InspectorHandler::registerCommand(
      "force_subscription_management_reauthentication",
      "Force re-authentication for the subscription management view", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        MozillaVPN::instance()->profileFlow()->setForceReauthFlow(true);
        return QJsonObject();
      });

  InspectorHandler::registerCommand(
      "force_unsecured_network", "Force an unsecured network detection", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        MozillaVPN::instance()->networkWatcher()->unsecuredNetwork("Dummy",
                                                                   "Dummy");
        return QJsonObject();
      });

  InspectorHandler::registerCommand(
      "force_update_check", "Force a version update check", 1,
      [](InspectorHandler*, const QList<QByteArray>& arguments) {
        s_updateVersion = arguments[1];
        MozillaVPN::instance()->releaseMonitor()->runSoon();
        return QJsonObject();
      });

  InspectorHandler::registerCommand(
      "hard_reset", "Hard reset (wipe all settings).", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        MozillaVPN::instance()->hardReset();
        return QJsonObject();
      });

  InspectorHandler::registerCommand(
      "logout", "Logout the user", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        MozillaVPN::instance()->logout();
        return QJsonObject();
      });

  InspectorHandler::registerCommand(
      "messages", "Returns a list of the loaded messages ids", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        QJsonObject obj;

        AddonManager* am = AddonManager::instance();
        Q_ASSERT(am);

        QJsonArray messages;
        am->forEach([&](Addon* addon) {
          if (addon->type() == "message") {
            messages.append(addon->id());
          }
        });

        obj["value"] = messages;
        return obj;
      });

  InspectorHandler::registerCommand(
      "mockFreeTrial", "Force the UI to show 7 day trial on 1 year plan", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        s_mockFreeTrial = true;
        return QJsonObject();
      });

  InspectorHandler::registerCommand(
      "public_key", "Retrieve the public key of the current device", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        MozillaVPN* vpn = MozillaVPN::instance();
        Q_ASSERT(vpn);

        Keys* keys = vpn->keys();
        Q_ASSERT(keys);

        QJsonObject obj;
        obj["value"] = keys->publicKey();
        return obj;
      });

  InspectorHandler::registerCommand(
      "reset", "Reset the app", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        MozillaVPN* vpn = MozillaVPN::instance();
        Q_ASSERT(vpn);

        vpn->reset(true);
        ErrorHandler::instance()->hideAlert();

        SettingsHolder* settingsHolder = SettingsHolder::instance();
        Q_ASSERT(settingsHolder);

        // Extra cleanup for testing
        settingsHolder->setTelemetryPolicyShown(false);

        return QJsonObject();
      });

  InspectorHandler::registerCommand(
      "reset_addons", "Reset all the addons cleaning up the cache", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        AddonManager::instance()->reset();
        return QJsonObject();
      });

  InspectorHandler::registerCommand(
      "servers", "Returns a list of servers", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        QJsonObject obj;

        QJsonArray countryArray;
        ServerCountryModel* scm = MozillaVPN::instance()->serverCountryModel();
        for (const ServerCountry& country : scm->countries()) {
          QJsonArray cityArray;
          for (const QString& cityName : country.cities()) {
            const ServerCity& city = scm->findCity(country.code(), cityName);
            if (!city.initialized()) {
              continue;
            }
            QJsonObject cityObj;
            cityObj["name"] = city.name();
            cityObj["localizedName"] =
                ServerI18N::instance()->translateCityName(country.code(),
                                                          city.name());
            cityObj["code"] = city.code();
            cityArray.append(cityObj);
          }

          QJsonObject countryObj;
          countryObj["name"] = country.name();
          countryObj["localizedName"] =
              ServerI18N::instance()->translateCountryName(country.code(),
                                                           country.name());
          countryObj["code"] = country.code();
          countryObj["cities"] = cityArray;

          countryArray.append(countryObj);
        }

        obj["value"] = countryArray;
        return obj;
      });

  InspectorHandler::registerCommand(
      "set_glean_source_tags",
      "Set Glean Source Tags (supply a comma seperated list)", 1,
      [](InspectorHandler*, const QList<QByteArray>& arguments) {
        QStringList tags = QString(arguments[1]).split(',');
        emit MozillaVPN::instance()->setGleanSourceTags(tags);
        return QJsonObject();
      });

  InspectorHandler::registerCommand(
      "quit", "Quit the app", 0,
      [](InspectorHandler*, const QList<QByteArray>&) {
        MozillaVPN::instance()->connectionManager()->quit();
        return QJsonObject();
      });

  InspectorHandler::registerCommand(
      "force_connection_health",
      "Force VPN connection health stability. Possible values are: stable, "
      "unstable, nosignal",
      1, [](InspectorHandler*, const QList<QByteArray>& arguments) {
        QJsonObject obj;
        auto stabilityMode = arguments[1];

        if (stabilityMode == "stable") {
          MozillaVPN::instance()
              ->connectionHealth()
              ->overwriteStabilityForInspector(
                  ConnectionHealth::ConnectionStability::Stable);
        } else if (stabilityMode == "unstable") {
          MozillaVPN::instance()
              ->connectionHealth()
              ->overwriteStabilityForInspector(
                  ConnectionHealth::ConnectionStability::Unstable);
        } else if (stabilityMode == "nosignal") {
          MozillaVPN::instance()
              ->connectionHealth()
              ->overwriteStabilityForInspector(
                  ConnectionHealth::ConnectionStability::NoSignal);
        } else {
          obj["error"] = QString("Please enter a valid stability mode value.");
        }
        return QJsonObject();
      });
}

// static
QString MozillaVPN::appVersionForUpdate() {
  if (s_updateVersion.isEmpty()) {
    return Constants::versionString();
  }

  return s_updateVersion;
}

// static
void MozillaVPN::registerAddonApis() {
  AddonApi::setConstructorCallback([](AddonApi* addonApi) {
    QJSEngine* engine = QmlEngineHolder::instance()->engine();

    {
      QObject* obj = MozillaVPN::instance()->connectionManager();
      QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

      QJSValue value = engine->newQObject(obj);
      value.setPrototype(engine->newQMetaObject(&Controller::staticMetaObject));

      addonApi->insert("controller", QVariant::fromValue(value));
    }

    {
      QObject* obj = MozillaVPN::instance()->subscriptionData();
      QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

      QJSValue value = engine->newQObject(obj);
      value.setPrototype(
          engine->newQMetaObject(&SubscriptionData::staticMetaObject));

      addonApi->insert("subscriptionData", QVariant::fromValue(value));
    }

    {
      QObject* obj = MozillaVPN::instance();
      QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

      QJSValue value = engine->newQObject(obj);
      value.setPrototype(engine->newQMetaObject(&MozillaVPN::staticMetaObject));

      addonApi->insert("vpn", QVariant::fromValue(value));
    }
  });
}

// static
void MozillaVPN::registerExternalOperations() {
  ExternalOpHandler* eoh = ExternalOpHandler::instance();

  eoh->registerExternalOperation(
      OpAbout, []() { MozillaVPN::instance()->requestAbout(); });

  eoh->registerExternalOperation(OpActivate, []() {
    TaskScheduler::deleteTasks();
    TaskScheduler::scheduleTask(
        new TaskControllerAction(TaskControllerAction::eActivate));
  });

  eoh->registerExternalOperation(OpDeactivate, []() {
    TaskScheduler::deleteTasks();
    TaskScheduler::scheduleTask(
        new TaskControllerAction(TaskControllerAction::eDeactivate));
  });

  eoh->registerExternalOperation(OpNotificationClicked, []() {});

  eoh->registerExternalOperation(
      OpQuit, []() { MozillaVPN::instance()->connectionManager()->quit(); });
}

void MozillaVPN::ensureApplicationIdExists() {
#if defined(MZ_ANDROID) || defined(MZ_IOS)
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  if (!settingsHolder->hasInstallationId()) {
    QString uuid = mozilla::glean::session::installation_id.generateAndSet();
    settingsHolder->setInstallationId(uuid);
  }
#endif
}

void MozillaVPN::gleanSetDebugViewTag(QString tag) {
  MZGlean::setDebugViewTag(tag);
}

void MozillaVPN::gleanSetLogPings(bool flag) { MZGlean::setLogPings(flag); }
