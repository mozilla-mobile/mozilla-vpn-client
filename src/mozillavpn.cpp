/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozillavpn.h"
#include "constants.h"
#include "dnshelper.h"
#include "featurelist.h"
#include "features/featureappreview.h"
#include "features/featureinapppurchase.h"
#include "features/featureinappauth.h"
#include "features/featureinappaccountcreate.h"
#include "features/featuresharelogs.h"
#include <telemetry/gleansample.h>
#include "iaphandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"
#include "logoutobserver.h"
#include "models/device.h"
#include "networkrequest.h"
#include "qmlengineholder.h"
#include "settingsholder.h"
#include "tasks/accountandservers/taskaccountandservers.h"
#include "tasks/adddevice/taskadddevice.h"
#include "tasks/authenticate/taskauthenticate.h"
#include "tasks/captiveportallookup/taskcaptiveportallookup.h"
#include "tasks/controlleraction/taskcontrolleraction.h"
#include "tasks/function/taskfunction.h"
#include "tasks/heartbeat/taskheartbeat.h"
#include "tasks/products/taskproducts.h"
#include "tasks/removedevice/taskremovedevice.h"
#include "tasks/surveydata/tasksurveydata.h"
#include "tasks/sendfeedback/tasksendfeedback.h"
#include "tasks/createsupportticket/taskcreatesupportticket.h"
#include "tasks/getfeaturelist/taskgetfeaturelist.h"
#include "taskscheduler.h"
#include "update/versionapi.h"
#include "urlopener.h"

#ifdef MVPN_IOS
#  include "platforms/ios/iosdatamigration.h"
#  include "platforms/ios/iosutils.h"
#endif

#ifdef MVPN_ANDROID
#  include "platforms/android/androidiaphandler.h"
#  include "platforms/android/androidutils.h"
#endif

#ifdef MVPN_WINDOWS
#  include "platforms/windows/windowsdatamigration.h"
#endif

#ifdef MVPN_ANDROID
#  include "platforms/android/androiddatamigration.h"
#  include "platforms/android/androidvpnactivity.h"
#  include "platforms/android/androidutils.h"
#endif

#ifdef MVPN_ADJUST
#  include "adjust/adjusthandler.h"
#endif

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QScreen>
#include <QTimer>
#include <QUrl>

// in seconds, hide alerts
constexpr const uint32_t HIDE_ALERT_SEC = 4;

namespace {
Logger logger(LOG_MAIN, "MozillaVPN");
MozillaVPN* s_instance = nullptr;
}  // namespace

// static
MozillaVPN* MozillaVPN::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

// static
MozillaVPN* MozillaVPN::maybeInstance() { return s_instance; }

MozillaVPN::MozillaVPN() : m_private(new Private()) {
  MVPN_COUNT_CTOR(MozillaVPN);

  logger.debug() << "Creating MozillaVPN singleton";

  Q_ASSERT(!s_instance);
  s_instance = this;

#ifdef MVPN_ADJUST
  AdjustHandler::initialize();
#endif

  connect(&m_alertTimer, &QTimer::timeout, [this]() { setAlert(NoAlert); });

  connect(&m_periodicOperationsTimer, &QTimer::timeout, []() {
    TaskScheduler::scheduleTask(new TaskAccountAndServers());
    TaskScheduler::scheduleTask(new TaskCaptivePortalLookup());
    TaskScheduler::scheduleTask(new TaskHeartbeat());
    TaskScheduler::scheduleTask(new TaskSurveyData());
    TaskScheduler::scheduleTask(new TaskGetFeatureList());
  });

  connect(this, &MozillaVPN::stateChanged, [this]() {
    if (m_state != StateMain) {
      // We don't call deactivate() because that is meant to be used for
      // UI interactions only and it deletes all the pending tasks.
      TaskScheduler::scheduleTask(
          new TaskControllerAction(TaskControllerAction::eDeactivate));
    }
  });

  connect(&m_private->m_controller, &Controller::readyToUpdate,
          [this]() { setState(StateUpdateRequired); });

  connect(&m_private->m_controller, &Controller::readyToBackendFailure,
          [this]() {
            TaskScheduler::deleteTasks();
            setState(StateBackendFailure);
          });

  connect(&m_private->m_controller, &Controller::stateChanged, this,
          &MozillaVPN::controllerStateChanged);

  connect(&m_private->m_controller, &Controller::stateChanged,
          &m_private->m_statusIcon, &StatusIcon::stateChanged);

  connect(this, &MozillaVPN::stateChanged, &m_private->m_statusIcon,
          &StatusIcon::stateChanged);

  connect(&m_private->m_controller, &Controller::stateChanged,
          &m_private->m_connectionHealth,
          &ConnectionHealth::connectionStateChanged);

  connect(&m_private->m_controller, &Controller::stateChanged,
          &m_private->m_captivePortalDetection,
          &CaptivePortalDetection::stateChanged);

  connect(&m_private->m_connectionHealth, &ConnectionHealth::stabilityChanged,
          &m_private->m_captivePortalDetection,
          &CaptivePortalDetection::stateChanged);

  connect(SettingsHolder::instance(),
          &SettingsHolder::captivePortalAlertChanged,
          &m_private->m_captivePortalDetection,
          &CaptivePortalDetection::settingsChanged);

  connect(&m_private->m_controller, &Controller::stateChanged,
          &m_private->m_connectionDataHolder,
          &ConnectionDataHolder::stateChanged);

  connect(this, &MozillaVPN::stateChanged, &m_private->m_connectionDataHolder,
          &ConnectionDataHolder::stateChanged);

  if (FeatureInAppPurchase::instance()->isSupported()) {
    IAPHandler* iap = IAPHandler::createInstance();
    connect(iap, &IAPHandler::subscriptionStarted, this,
            &MozillaVPN::subscriptionStarted);
    connect(iap, &IAPHandler::subscriptionFailed, this,
            &MozillaVPN::subscriptionFailed);
    connect(iap, &IAPHandler::subscriptionCanceled, this,
            &MozillaVPN::subscriptionCanceled);
    connect(iap, &IAPHandler::subscriptionCompleted, this,
            &MozillaVPN::subscriptionCompleted);
    connect(iap, &IAPHandler::alreadySubscribed, this,
            &MozillaVPN::alreadySubscribed);
    connect(iap, &IAPHandler::billingNotAvailable, this,
            &MozillaVPN::billingNotAvailable);
    connect(iap, &IAPHandler::subscriptionNotValidated, this,
            &MozillaVPN::subscriptionNotValidated);
  }
}

MozillaVPN::~MozillaVPN() {
  MVPN_COUNT_DTOR(MozillaVPN);

  logger.debug() << "Deleting MozillaVPN singleton";

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;

  delete m_private;
}

MozillaVPN::State MozillaVPN::state() const { return m_state; }

bool MozillaVPN::stagingMode() const { return !Constants::inProduction(); }

bool MozillaVPN::debugMode() const {
#ifdef MVPN_DEBUG
  return true;
#else
  return false;
#endif
}

void MozillaVPN::initialize() {
  logger.debug() << "MozillaVPN Initialization";

  Q_ASSERT(!m_initialized);
  m_initialized = true;

  // This is our first state.
  Q_ASSERT(m_state == StateInitialize);

  m_private->m_releaseMonitor.runSoon();

  TaskScheduler::scheduleTask(new TaskGetFeatureList());

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

#ifdef MVPN_IOS
  if (!settingsHolder->nativeIOSDataMigrated()) {
    IOSDataMigration::migrate();
    settingsHolder->setNativeIOSDataMigrated(true);
  }
#endif

#ifdef MVPN_WINDOWS
  if (!settingsHolder->nativeWindowsDataMigrated()) {
    WindowsDataMigration::migrate();
    settingsHolder->setNativeWindowsDataMigrated(true);
  }
#endif

#ifdef MVPN_ANDROID
  if (!settingsHolder->nativeAndroidDataMigrated()) {
    AndroidDataMigration::migrate();
    settingsHolder->setNativeAndroidDataMigrated(true);
  }
  AndroidVPNActivity::init();
#endif

  m_private->m_captivePortalDetection.initialize();
  m_private->m_networkWatcher.initialize();

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
  if (FeatureInAppPurchase::instance()->isSupported()) {
    if (m_private->m_user.subscriptionNeeded()) {
      setUserState(UserAuthenticated);
      setState(StateAuthenticating);
      TaskScheduler::scheduleTask(new TaskProducts());
      TaskScheduler::scheduleTask(
          new TaskFunction([this]() { maybeStateMain(); }));
      return;
    }
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

  if (!m_private->m_deviceModel.hasCurrentDevice(keys())) {
    logger.error() << "The current device has not been found";
    settingsHolder->clear();
    return;
  }

  if (!m_private->m_captivePortal.fromSettings()) {
    // We do not care about CaptivePortal settings.
  }

  if (!m_private->m_surveyModel.fromSettings()) {
    // We do not care about Survey settings.
  }

  if (!modelsInitialized()) {
    logger.error() << "Models not initialized yet";
    settingsHolder->clear();
    return;
  }

  Q_ASSERT(!m_private->m_serverData.initialized());
  if (!m_private->m_serverData.fromSettings()) {
    m_private->m_serverCountryModel.pickRandom(m_private->m_serverData);
    Q_ASSERT(m_private->m_serverData.initialized());
    m_private->m_serverData.writeSettings();
  }

  TaskScheduler::scheduleTask(new TaskAccountAndServers());
  TaskScheduler::scheduleTask(new TaskCaptivePortalLookup());
  TaskScheduler::scheduleTask(new TaskSurveyData());

  if (FeatureInAppPurchase::instance()->isSupported()) {
    TaskScheduler::scheduleTask(new TaskProducts());
  }

  setUserState(UserAuthenticated);
  maybeStateMain();
}

void MozillaVPN::setState(State state) {
  logger.debug() << "Set state:" << state;

  m_state = state;
  emit stateChanged();

  // If we are activating the app, let's initialize the controller.
  if (m_state == StateMain) {
    m_private->m_controller.initialize();
    startSchedulingPeriodicOperations();
  } else {
    stopSchedulingPeriodicOperations();
  }
}

void MozillaVPN::maybeStateMain() {
  logger.debug() << "Maybe state main";

  if (FeatureInAppPurchase::instance()->isSupported()) {
    if (m_state != StateSubscriptionBlocked &&
        m_private->m_user.subscriptionNeeded()) {
      logger.info() << "Subscription needed";
      setState(StateSubscriptionNeeded);
      return;
    }
    if (m_state == StateSubscriptionBlocked) {
      logger.info() << "Subscription is blocked, stay blocked.";
      return;
    }
  }

  SettingsHolder* settingsHolder = SettingsHolder::instance();

#if !defined(MVPN_ANDROID) && !defined(MVPN_IOS)
  if (!settingsHolder->postAuthenticationShown()) {
    setState(StatePostAuthentication);
    return;
  }
#endif

  if (!settingsHolder->telemetryPolicyShown()) {
    setState(StateTelemetryPolicy);
    return;
  }

  if (!m_private->m_deviceModel.hasCurrentDevice(keys())) {
    Q_ASSERT(m_private->m_deviceModel.activeDevices() ==
             m_private->m_user.maxDevices());
    emit recordGleanEvent(GleanSample::maxDeviceReached);
    setState(StateDeviceLimit);
    return;
  }

  // For 2.5 we need to regenerate the device key to allow the the custom DNS
  // feature. We can do it in background when the main view is shown.
  maybeRegenerateDeviceKey();

  if (m_state != StateUpdateRequired) {
    setState(StateMain);
  }

#ifdef MVPN_ADJUST
  // When the client is ready to be activated, we do not need adjustSDK anymore
  // (the subscription is done, and no extra events will be dispatched). We
  // cannot disable AdjustSDK at runtime, but we can disable it for the next
  // execution.
  if (settingsHolder->hasAdjustActivatable()) {
    settingsHolder->setAdjustActivatable(false);
  }
#endif
}

void MozillaVPN::setServerPublicKey(const QString& publicKey) {
  logger.debug() << "Set server public key:" << publicKey;
  m_serverPublicKey = publicKey;
}

void MozillaVPN::getStarted() {
  logger.debug() << "Get started";
  authenticate();
}

void MozillaVPN::authenticate() {
  return authenticateWithType(FeatureInAppAuth::instance()->isSupported()
                                  ? AuthenticationInApp
                                  : AuthenticationInBrowser);
}

void MozillaVPN::authenticateWithType(
    MozillaVPN::AuthenticationType authenticationType) {
  logger.debug() << "Authenticate";

  setState(StateAuthenticating);

  hideAlert();

  if (m_userState != UserNotAuthenticated) {
    // If we try to start an authentication flow when already logged in, there
    // is a bug elsewhere.
    Q_ASSERT(m_userState == UserLoggingOut);

    LogoutObserver* lo = new LogoutObserver(this);
    // Let's use QueuedConnection to avoid nexted tasks executions.
    connect(
        lo, &LogoutObserver::ready, this,
        [this, authenticationType]() {
          authenticateWithType(authenticationType);
        },
        Qt::QueuedConnection);
    return;
  }

  emit recordGleanEvent(GleanSample::authenticationStarted);

  TaskScheduler::scheduleTask(new TaskHeartbeat());
  TaskScheduler::scheduleTask(new TaskAuthenticate(authenticationType));
}

void MozillaVPN::abortAuthentication() {
  logger.warning() << "Abort authentication";
  Q_ASSERT(m_state == StateAuthenticating);
  setState(StateInitialize);

  emit recordGleanEvent(GleanSample::authenticationAborted);
}

void MozillaVPN::openLink(LinkType linkType) {
  logger.debug() << "Opening link: " << linkType;

  QString url;
  bool addEmailAddress = false;

  switch (linkType) {
    case LinkAccount:
      url = NetworkRequest::apiBaseUrl();
      url.append("/r/vpn/account");
      addEmailAddress = true;
      break;

    case LinkContact:
      url = NetworkRequest::apiBaseUrl();
      url.append("/r/vpn/contact");
      break;

    case LinkFeedback:
      url = NetworkRequest::apiBaseUrl();
      url.append("/r/vpn/client/feedback");
      break;

    case LinkHelpSupport:
      url = NetworkRequest::apiBaseUrl();
      url.append("/r/vpn/support");
      break;

    case LinkLeaveReview:
      Q_ASSERT(FeatureAppReview::instance()->isSupported());
      url =
#if defined(MVPN_IOS)
          "https://apps.apple.com/app/id1489407738?action=write-review";
#elif defined(MVPN_ANDROID)
          "https://play.google.com/store/apps/"
          "details?id=org.mozilla.firefox.vpn";
#else
          "";
#endif
      break;

    case LinkTermsOfService:
      url = NetworkRequest::apiBaseUrl();
      url.append("/r/vpn/terms");
      break;

    case LinkPrivacyNotice:
      url = NetworkRequest::apiBaseUrl();
      url.append("/r/vpn/privacy");
      break;

    case LinkUpdate:
      url = NetworkRequest::apiBaseUrl();
      url.append("/r/vpn/update/");
      url.append(Constants::PLATFORM_NAME);
      break;

    case LinkSubscriptionBlocked:
      url = NetworkRequest::apiBaseUrl();
      url.append("/r/vpn/subscriptionBlocked");
      break;
    case LinkSplitTunnelHelp:
      // TODO: This should link to a more helpful article
      url =
          "https://support.mozilla.org/kb/"
          "split-tunneling-use-mozilla-vpn-specific-apps-wind";
      break;
    case LinkInspector:
      Q_ASSERT(!Constants::inProduction());
      url = "http://localhost:8766/";
      break;

    default:
      qFatal("Unsupported link type!");
      return;
  }

  UrlOpener::open(url, addEmailAddress);
}

void MozillaVPN::openLinkUrl(const QString& linkUrl) {
  logger.debug() << "Opening link: " << linkUrl;
  UrlOpener::open(linkUrl);
}

void MozillaVPN::setToken(const QString& token) {
  SettingsHolder::instance()->setToken(token);
}

void MozillaVPN::authenticationCompleted(const QByteArray& json,
                                         const QString& token) {
  logger.debug() << "Authentication completed";

  emit recordGleanEvent(GleanSample::authenticationCompleted);

  if (!m_private->m_user.fromJson(json)) {
    logger.error() << "Failed to parse the User JSON data";
    errorHandle(ErrorHandler::RemoteServiceError);
    return;
  }

  if (!m_private->m_deviceModel.fromJson(keys(), json)) {
    logger.error() << "Failed to parse the DeviceModel JSON data";
    errorHandle(ErrorHandler::RemoteServiceError);
    return;
  }

  m_private->m_user.writeSettings();
  m_private->m_deviceModel.writeSettings();

  setToken(token);
  setUserState(UserAuthenticated);

  if (FeatureInAppPurchase::instance()->isSupported()) {
    if (m_private->m_user.subscriptionNeeded()) {
      TaskScheduler::scheduleTask(new TaskProducts());
      TaskScheduler::scheduleTask(
          new TaskFunction([this]() { maybeStateMain(); }));
      return;
    }
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

  if (deviceCount >= m_private->m_user.maxDevices()) {
    maybeStateMain();
    return;
  }

  // Here we add the current device.
  if (m_private->m_keys.privateKey().isEmpty() ||
      !m_private->m_deviceModel.hasCurrentDevice(keys())) {
    addCurrentDeviceAndRefreshData();
  } else {
    // Let's fetch the account and the servers.
    TaskScheduler::scheduleTask(new TaskAccountAndServers());
  }

  if (FeatureInAppPurchase::instance()->isSupported()) {
    TaskScheduler::scheduleTask(new TaskProducts());
  }

  TaskScheduler::scheduleTask(new TaskSurveyData());

  // Finally we are able to activate the client.
  TaskScheduler::scheduleTask(new TaskFunction([this]() {
    if (!modelsInitialized()) {
      logger.error() << "Failed to complete the authentication";
      errorHandle(ErrorHandler::RemoteServiceError);
      setUserState(UserNotAuthenticated);
      return;
    }

    Q_ASSERT(m_private->m_serverData.initialized());

    maybeStateMain();
  }));
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

  settingsHolder->setDeviceKeyVersion(APP_VERSION);
}

void MozillaVPN::deviceRemoved(const QString& publicKey) {
  logger.debug() << "Device removed";

  m_private->m_deviceModel.removeDeviceFromPublicKey(publicKey);
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
  if (!m_private->m_serverData.initialized() ||
      !m_private->m_serverCountryModel.exists(m_private->m_serverData)) {
    m_private->m_serverCountryModel.pickRandom(m_private->m_serverData);
    Q_ASSERT(m_private->m_serverData.initialized());
    m_private->m_serverData.writeSettings();
  }
}

void MozillaVPN::deviceRemovalCompleted(const QString& publicKey) {
  logger.debug() << "Device removal task completed";
  m_private->m_deviceModel.stopDeviceRemovalFromPublicKey(publicKey, keys());
}

void MozillaVPN::removeDeviceFromPublicKey(const QString& publicKey) {
  logger.debug() << "Remove device";

  const Device* device =
      m_private->m_deviceModel.deviceFromPublicKey(publicKey);
  if (device) {
    // Let's inform the UI about what is going to happen.
    emit deviceRemoving(publicKey);
    TaskScheduler::scheduleTask(new TaskRemoveDevice(publicKey));

    if (m_state != StateDeviceLimit) {
      // To have a faster UI, we inform the device-model that this public key
      // is going to be removed.
      m_private->m_deviceModel.startDeviceRemovalFromPublicKey(publicKey);
    }
  }

  if (m_state != StateDeviceLimit) {
    return;
  }

  // Let's recover from the device-limit mode.
  Q_ASSERT(!m_private->m_deviceModel.hasCurrentDevice(keys()));

  // Here we add the current device.
  addCurrentDeviceAndRefreshData();

  // Finally we are able to activate the client.
  TaskScheduler::scheduleTask(new TaskFunction([this]() {
    if (m_state != StateDeviceLimit) {
      return;
    }

    if (!modelsInitialized()) {
      logger.warning() << "Models not initialized yet";
      errorHandle(ErrorHandler::RemoteServiceError);
      SettingsHolder::instance()->clear();
      setUserState(UserNotAuthenticated);
      setState(StateInitialize);
      return;
    }

    maybeStateMain();
  }));
}

void MozillaVPN::submitFeedback(const QString& feedbackText, const qint8 rating,
                                const QString& category) {
  logger.debug() << "Submit Feedback";

  QString* buffer = new QString();
  QTextStream* out = new QTextStream(buffer);

  serializeLogs(out, [out, buffer, feedbackText, rating, category] {
    Q_ASSERT(out);
    Q_ASSERT(buffer);

    // buffer is getting copied by TaskSendFeedback so we can delete it
    // afterwards
    TaskScheduler::scheduleTask(
        new TaskSendFeedback(feedbackText, *buffer, rating, category));

    delete buffer;
    delete out;
  });
}

void MozillaVPN::createSupportTicket(const QString& email,
                                     const QString& subject,
                                     const QString& issueText,
                                     const QString& category) {
  logger.debug() << "Create support ticket";

  QString* buffer = new QString();
  QTextStream* out = new QTextStream(buffer);

  serializeLogs(out, [out, buffer, email, subject, issueText, category] {
    Q_ASSERT(out);
    Q_ASSERT(buffer);

    // buffer is getting copied by TaskCreateSupportTicket so we can delete it
    // afterwards
    TaskScheduler::scheduleTask(new TaskCreateSupportTicket(
        email, subject, issueText, *buffer, category));

    delete buffer;
    delete out;
  });
}

#ifdef MVPN_ANDROID
void MozillaVPN::launchPlayStore() {
  logger.debug() << "Launch Play Store";
  IAPHandler* iap = IAPHandler::instance();
  static_cast<AndroidIAPHandler*>(iap)->launchPlayStore();
}
#endif

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

  m_private->m_user.writeSettings();
  m_private->m_deviceModel.writeSettings();

  if (FeatureInAppPurchase::instance()->isSupported()) {
    if (m_private->m_user.subscriptionNeeded() && m_state == StateMain) {
      maybeStateMain();
      return;
    }
  }

  // To test the subscription needed view, comment out this line:
  // m_private->m_controller.subscriptionNeeded();
}

void MozillaVPN::surveyChecked(const QByteArray& json) {
  logger.debug() << "Survey checked";

  if (!m_private->m_surveyModel.fromJson(json)) {
    logger.error() << "Failed to parse the Survey JSON data";
    return;
  }

  m_private->m_surveyModel.writeSettings();
}

void MozillaVPN::cancelAuthentication() {
  logger.warning() << "Canceling authentication";

  if (m_state != StateAuthenticating) {
    // We cannot cancel tasks if we are not in authenticating state.
    return;
  }

  emit recordGleanEvent(GleanSample::authenticationAborted);

  reset(true);
}

void MozillaVPN::logout() {
  logger.debug() << "Logout";

  setAlert(LogoutAlert);
  setUserState(UserLoggingOut);

  TaskScheduler::deleteTasks();

  if (FeatureInAppPurchase::instance()->isSupported()) {
    IAPHandler* iap = IAPHandler::instance();
    iap->stopSubscription();
    iap->stopProductsRegistration();
  }

  // update-required state is the only one we want to keep when logging out.
  if (m_state != StateUpdateRequired) {
    setState(StateInitialize);
  }

  if (m_private->m_deviceModel.hasCurrentDevice(keys())) {
    TaskScheduler::scheduleTask(new TaskRemoveDevice(keys()->publicKey()));
  }

  TaskScheduler::scheduleTask(new TaskFunction([this]() { reset(false); }));
}

void MozillaVPN::reset(bool forceInitialState) {
  logger.debug() << "Cleaning up all";

  TaskScheduler::deleteTasks();

  SettingsHolder::instance()->clear();
  m_private->m_keys.forgetKeys();
  m_private->m_serverData.forget();

  if (FeatureInAppPurchase::instance()->isSupported()) {
    IAPHandler* iap = IAPHandler::instance();
    iap->stopSubscription();
    iap->stopProductsRegistration();
  }

  setUserState(UserNotAuthenticated);

  if (forceInitialState) {
    setState(StateInitialize);
  }
}

void MozillaVPN::setAlert(AlertType alert) {
  m_alertTimer.stop();

  if (alert != NoAlert) {
    m_alertTimer.start(1000 * HIDE_ALERT_SEC);
  }

  m_alert = alert;
  emit alertChanged();
}

void MozillaVPN::errorHandle(ErrorHandler::ErrorType error) {
  logger.debug() << "Handling error" << error;

  Q_ASSERT(error != ErrorHandler::NoError);

  AlertType alert = NoAlert;

  switch (error) {
    case ErrorHandler::VPNDependentConnectionError:
      if (controller()->state() == Controller::State::StateOn ||
          controller()->state() == Controller::State::StateConfirming) {
        // connection likely isn't stable yet
        logger.error() << "Ignore network error probably caused by enabled VPN";
        return;
      } else if (controller()->state() == Controller::State::StateOff) {
        // We are off, so this means a request failed, not the
        // VPN. Change it to No Connection
        alert = NoConnectionAlert;
        break;
      }
      [[fallthrough]];
    case ErrorHandler::ConnectionFailureError:
      alert = ConnectionFailedAlert;
      break;

    case ErrorHandler::NoConnectionError:
      if (controller()->isUnsettled()) {
        return;
      }
      alert = NoConnectionAlert;
      break;

    case ErrorHandler::AuthenticationError:
      alert = AuthenticationFailedAlert;
      break;

    case ErrorHandler::ControllerError:
      alert = ControllerErrorAlert;
      break;

    case ErrorHandler::RemoteServiceError:
      alert = RemoteServiceErrorAlert;
      break;

    case ErrorHandler::SubscriptionFailureError:
      alert = SubscriptionFailureAlert;
      break;

    case ErrorHandler::GeoIpRestrictionError:
      alert = GeoIpRestrictionAlert;
      break;

    case ErrorHandler::UnrecoverableError:
      alert = UnrecoverableErrorAlert;
      break;

    default:
      break;
  }

  setAlert(alert);

  logger.error() << "Alert:" << alert << "State:" << m_state;

  if (alert == NoAlert) {
    return;
  }

  // Any error in authenticating state sends to the Initial state.
  if (m_state == StateAuthenticating) {
    if (alert == GeoIpRestrictionAlert) {
      emit recordGleanEvent(GleanSample::authenticationFailureByGeo);
    } else {
      emit recordGleanEvent(GleanSample::authenticationFailure);
    }
    setState(StateInitialize);
    return;
  }

  if (alert == AuthenticationFailedAlert) {
    reset(true);
    return;
  }
}

const QList<Server> MozillaVPN::exitServers() const {
  return m_private->m_serverCountryModel.servers(m_private->m_serverData);
}

const QList<Server> MozillaVPN::entryServers() const {
  if (!m_private->m_serverData.multihop()) {
    return QList<Server>();
  }
  ServerData sd;
  sd.update(m_private->m_serverData.entryCountryCode(),
            m_private->m_serverData.entryCityName());
  return m_private->m_serverCountryModel.servers(sd);
}

void MozillaVPN::changeServer(const QString& countryCode, const QString& city,
                              const QString& entryCountryCode,
                              const QString& entryCity) {
  m_private->m_serverData.update(countryCode, city, entryCountryCode,
                                 entryCity);
  m_private->m_serverData.writeSettings();

  // Update the list of recent connections.
  QString description = m_private->m_serverData.toString();
  QStringList recent = SettingsHolder::instance()->recentConnections();
  int index = recent.indexOf(description);
  if (index == 0) {
    // This is already the most-recent connection.
    return;
  }

  if (index > 0) {
    recent.removeAt(index);
  } else {
    while (recent.count() >= Constants::RECENT_CONNECTIONS_MAX_COUNT) {
      recent.removeLast();
    }
  }
  recent.prepend(description);
  SettingsHolder::instance()->setRecentConnections(recent);
}

void MozillaVPN::postAuthenticationCompleted() {
  logger.debug() << "Post authentication completed";

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  settingsHolder->setPostAuthenticationShown(true);

  // Super racy, but it could happen that we are already in update-required
  // state.
  if (m_state == StateUpdateRequired) {
    return;
  }

  maybeStateMain();
}

void MozillaVPN::mainWindowLoaded() {
  logger.debug() << "main window loaded";

#ifndef MVPN_WASM
  // Initialize glean
  logger.debug() << "Initializing Glean";
  emit initializeGlean();

  // Setup regular glean ping sending
  connect(&m_gleanTimer, &QTimer::timeout, this, &MozillaVPN::sendGleanPings);
  m_gleanTimer.start(Constants::gleanTimeoutMsec());
  m_gleanTimer.setSingleShot(false);
#endif
}

void MozillaVPN::telemetryPolicyCompleted() {
  logger.debug() << "telemetry policy completed";

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  settingsHolder->setTelemetryPolicyShown(true);

  // Super racy, but it could happen that we are already in update-required
  // state.
  if (m_state == StateUpdateRequired) {
    return;
  }

  if (m_userState != UserAuthenticated) {
    authenticate();
    return;
  }

  maybeStateMain();
}

void MozillaVPN::setUpdateRecommended(bool value) {
  m_updateRecommended = value;
  emit updateRecommendedChanged();
}

void MozillaVPN::setUserState(UserState state) {
  logger.debug() << "User authentication state:" << state;
  if (m_userState != state) {
    m_userState = state;
    emit userStateChanged();
  }
}

void MozillaVPN::startSchedulingPeriodicOperations() {
  logger.debug() << "Start scheduling account and servers"
                 << Constants::scheduleAccountAndServersTimerMsec();
  m_periodicOperationsTimer.start(
      Constants::scheduleAccountAndServersTimerMsec());
}

void MozillaVPN::stopSchedulingPeriodicOperations() {
  logger.debug() << "Stop scheduling account and servers";
  m_periodicOperationsTimer.stop();
}

bool MozillaVPN::writeAndShowLogs(QStandardPaths::StandardLocation location) {
  return writeLogs(location, [](const QString& filename) {
    logger.debug() << "Opening the logFile somehow:" << filename;
    QUrl url = QUrl::fromLocalFile(filename);
    UrlOpener::open(url);
  });
}

bool MozillaVPN::writeLogs(
    QStandardPaths::StandardLocation location,
    std::function<void(const QString& filename)>&& a_callback) {
  logger.debug() << "Trying to save logs in:" << location;

  std::function<void(const QString& filename)> callback = std::move(a_callback);

  if (!QFileInfo::exists(QStandardPaths::writableLocation(location))) {
    return false;
  }

  QString filename;
  QDate now = QDate::currentDate();

  QTextStream(&filename) << "mozillavpn-" << now.year() << "-" << now.month()
                         << "-" << now.day() << ".txt";

  QDir logDir(QStandardPaths::writableLocation(location));
  QString logFile = logDir.filePath(filename);

  if (QFileInfo::exists(logFile)) {
    logger.warning() << logFile << "exists. Let's try a new filename";

    for (uint32_t i = 1;; ++i) {
      QString filename;
      QTextStream(&filename)
          << "mozillavpn-" << now.year() << "-" << now.month() << "-"
          << now.day() << "_" << i << ".txt";
      logFile = logDir.filePath(filename);
      if (!QFileInfo::exists(logFile)) {
        logger.debug() << "Filename found!" << i;
        break;
      }
    }
  }

  logger.debug() << "Writing logs into: " << logFile;

  QFile* file = new QFile(logFile);
  if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
    logger.error() << "Failed to open the logfile";
    delete file;
    return false;
  }

  QTextStream* out = new QTextStream(file);
  serializeLogs(out, [callback = std::move(callback), logFile, file, out]() {
    Q_ASSERT(out);
    Q_ASSERT(file);
    delete out;
    delete file;

    callback(logFile);
  });

  return true;
}

void MozillaVPN::serializeLogs(QTextStream* out,
                               std::function<void()>&& a_finalizeCallback) {
  std::function<void()> finalizeCallback = std::move(a_finalizeCallback);

  *out << "Mozilla VPN logs" << Qt::endl
       << "================" << Qt::endl
       << Qt::endl;

  LogHandler::writeLogs(*out);

  MozillaVPN::instance()->controller()->getBackendLogs(
      [out,
       finalizeCallback = std::move(finalizeCallback)](const QString& logs) {
        logger.debug() << "Logs from the backend service received";

        *out << Qt::endl
             << Qt::endl
             << "Mozilla VPN backend logs" << Qt::endl
             << "========================" << Qt::endl
             << Qt::endl;

        if (!logs.isEmpty()) {
          *out << logs;
        } else {
          *out << "No logs from the backend.";
        }
        *out << Qt::endl;
        *out << "==== SETTINGS ====" << Qt::endl;
        *out << SettingsHolder::instance()->getReport();
        *out << "==== DEVICE ====" << Qt::endl;
        *out << Device::currentDeviceReport();
        *out << Qt::endl;

        finalizeCallback();
      });
}

bool MozillaVPN::viewLogs() {
  logger.debug() << "View logs";

  if (!FeatureShareLogs::instance()->isSupported()) {
    logger.error() << "ViewLogs Called on unsupported OS or version!";
    return false;
  }

#if defined(MVPN_ANDROID) || defined(MVPN_IOS)
  QString* buffer = new QString();
  QTextStream* out = new QTextStream(buffer);
  bool ok = true;
  serializeLogs(out, [buffer, out
#  if defined(MVPN_ANDROID)
                      ,
                      &ok
#  endif
  ]() {
    Q_ASSERT(out);
    Q_ASSERT(buffer);

#  if defined(MVPN_ANDROID)
    ok = AndroidUtils::ShareText(*buffer);
#  else
    IOSUtils::shareLogs(*buffer);
#  endif

    delete out;
    delete buffer;
  });
  return ok;
#endif

  if (writeAndShowLogs(QStandardPaths::DesktopLocation)) {
    return true;
  }

  if (writeAndShowLogs(QStandardPaths::HomeLocation)) {
    return true;
  }

  if (writeAndShowLogs(QStandardPaths::TempLocation)) {
    return true;
  }

  qWarning()
      << "No Desktop, no Home, no Temp folder. Unable to store the log files.";
  return false;
}

void MozillaVPN::retrieveLogs() {
  logger.debug() << "Retrieve logs";

  QString* buffer = new QString();
  QTextStream* out = new QTextStream(buffer);

  serializeLogs(out, [this, buffer, out]() {
    Q_ASSERT(out);
    Q_ASSERT(buffer);

    delete out;
    emit logsReady(*buffer);
    delete buffer;
  });
}

void MozillaVPN::storeInClipboard(const QString& text) {
  logger.debug() << "Store in clipboard";
  QApplication::clipboard()->setText(text);
}

void MozillaVPN::cleanupLogs() {
  logger.debug() << "Cleanup logs";
  LogHandler::instance()->cleanupLogs();
  MozillaVPN::instance()->controller()->cleanupBackendLogs();
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

void MozillaVPN::requestSettings() {
  logger.debug() << "Settings required";

  QmlEngineHolder::instance()->showWindow();
  emit settingsNeeded();
}

void MozillaVPN::requestAbout() {
  logger.debug() << "About view requested";

  QmlEngineHolder::instance()->showWindow();
  emit aboutNeeded();
}

void MozillaVPN::requestViewLogs() {
  logger.debug() << "View log requested";

  QmlEngineHolder::instance()->showWindow();
  emit viewLogsNeeded();
}

void MozillaVPN::requestContactUs() {
  logger.debug() << "Contact us view requested";

  QmlEngineHolder::instance()->showWindow();
  emit contactUsNeeded();
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

void MozillaVPN::deactivate() {
  logger.debug() << "VPN tunnel deactivation";

  TaskScheduler::deleteTasks();
  TaskScheduler::scheduleTask(
      new TaskControllerAction(TaskControllerAction::eDeactivate));
}

void MozillaVPN::silentSwitch() {
  logger.debug() << "VPN tunnel silent server switch";

  TaskScheduler::scheduleTask(
      new TaskControllerAction(TaskControllerAction::eSilentSwitch));
}

void MozillaVPN::refreshDevices() {
  logger.debug() << "Refresh devices";

  if (m_state == StateMain) {
    TaskScheduler::scheduleTask(new TaskAccountAndServers());
  }
}

void MozillaVPN::quit() {
  logger.debug() << "quit";
  TaskScheduler::deleteTasks();
  qApp->quit();
}

void MozillaVPN::subscriptionStarted(const QString& productIdentifier) {
  logger.debug() << "Subscription started" << productIdentifier;

  setState(StateSubscriptionInProgress);

  IAPHandler* iap = IAPHandler::instance();

  // If IAP is not ready (race condition), register the products again.
  if (!iap->hasProductsRegistered()) {
    TaskScheduler::scheduleTask(new TaskProducts());
    TaskScheduler::scheduleTask(new TaskFunction([this, productIdentifier]() {
      subscriptionStarted(productIdentifier);
    }));

    return;
  }

  iap->startSubscription(productIdentifier);
}

void MozillaVPN::subscriptionCompleted() {
#ifdef MVPN_ANDROID
  // This is Android only
  // iOS can end up here if the subsciption get finished outside of the IAP
  // process
  if (m_state != StateSubscriptionInProgress) {
    // We could hit this in android flow if we're doing a late acknowledgement.
    // And ignoring is fine.
    logger.warning()
        << "Random subscription completion received. Let's ignore it.";
    return;
  }
#endif

  logger.debug() << "Subscription completed";

#ifdef MVPN_ADJUST
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
#ifdef MVPN_IOS
  // This is iOS only.
  // Android can legitimately end up here on a skuDetailsFailed.
  if (m_state != StateSubscriptionInProgress) {
    logger.warning()
        << "Random subscription failure received. Let's ignore it.";
    return;
  }
#endif

  logger.debug() << "Subscription failed or canceled";

  // Let's go back to the subscription needed.
  setState(StateSubscriptionNeeded);

  if (!canceledByUser) {
    errorHandle(ErrorHandler::SubscriptionFailureError);
  }

  TaskScheduler::scheduleTask(new TaskAccountAndServers());
  TaskScheduler::scheduleTask(new TaskFunction([this]() {
    if (!m_private->m_user.subscriptionNeeded() &&
        m_state == StateSubscriptionNeeded) {
      maybeStateMain();
      return;
    }
  }));
}

void MozillaVPN::alreadySubscribed() {
#ifdef MVPN_IOS
  // This randomness is an iOS only issue
  // TODO - How can we make this cleaner in the future
  if (m_state != StateSubscriptionInProgress) {
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
#ifndef MVPN_WINDOWS
  if (m_private->m_controller.state() != Controller::StateOff &&
      m_private->m_controller.state() != Controller::StateInitializing) {
    deactivate();
    return;
  }
#endif

  m_private->m_releaseMonitor.update();
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

  if (m_updating && m_private->m_controller.state() == Controller::StateOff) {
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

  if (!success) {
    m_private->m_controller.backendFailure();
    return;
  }

  if (m_state != StateBackendFailure) {
    return;
  }

  if (!modelsInitialized() || m_userState != UserAuthenticated) {
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
  TaskScheduler::scheduleTask(new TaskAccountAndServers());
}

void MozillaVPN::openAppStoreReviewLink() {
  Q_ASSERT(FeatureAppReview::instance()->isSupported());
  openLink(LinkType::LinkLeaveReview);
}

bool MozillaVPN::validateUserDNS(const QString& dns) const {
  return DNSHelper::validateUserDNS(dns);
}

void MozillaVPN::maybeRegenerateDeviceKey() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  if (settingsHolder->hasDeviceKeyVersion() &&
      VersionApi::compareVersions(settingsHolder->deviceKeyVersion(),
                                  APP_VERSION) >= 0) {
    return;
  }

  // We need a new device key only if the user wants to use custom DNS servers.
  if (settingsHolder->dnsProvider() == SettingsHolder::DnsProvider::Gateway) {
    logger.debug() << "Removal needed but no custom DNS used.";
    return;
  }

  Q_ASSERT(m_private->m_deviceModel.hasCurrentDevice(keys()));

  logger.debug() << "Removal needed for the 2.5 key regeneration.";

  // We do not need to remove the current device! guardian-website "overwrites"
  // the current device key when we submit a new one.
  addCurrentDeviceAndRefreshData();
}

void MozillaVPN::hardReset() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);
  settingsHolder->hardReset();
}

void MozillaVPN::hardResetAndQuit() {
  logger.debug() << "Hard reset and quit";
  hardReset();
  quit();
}
