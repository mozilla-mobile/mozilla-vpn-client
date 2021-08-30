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
#include "gleansample.h"
#include "iaphandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"
#include "logoutobserver.h"
#include "models/device.h"
#include "models/servercountrymodel.h"
#include "models/user.h"
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
#include "urlopener.h"

#ifdef MVPN_IOS
#  include "platforms/ios/iosdatamigration.h"
#  include "platforms/ios/iosadjusthelper.h"
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

MozillaVPN::MozillaVPN() : m_private(new Private()) {
  MVPN_COUNT_CTOR(MozillaVPN);

  logger.debug() << "Creating MozillaVPN singleton";

#ifdef MVPN_IOS
  IOSAdjustHelper::initialize();
#endif

  Q_ASSERT(!s_instance);
  s_instance = this;

  connect(&m_alertTimer, &QTimer::timeout, [this]() { setAlert(NoAlert); });

  connect(&m_periodicOperationsTimer, &QTimer::timeout, [this]() {
    scheduleTask(new TaskAccountAndServers());
    scheduleTask(new TaskCaptivePortalLookup());
    scheduleTask(new TaskHeartbeat());
    scheduleTask(new TaskSurveyData());
    scheduleTask(new TaskGetFeatureList());
  });

  connect(this, &MozillaVPN::stateChanged, [this]() {
    if (m_state != StateMain) {
      // We don't call deactivate() because that is meant to be used for
      // UI interactions only and it deletes all the pending tasks.
      scheduleTask(new TaskControllerAction(TaskControllerAction::eDeactivate));
    }
  });

  connect(&m_private->m_controller, &Controller::readyToUpdate,
          [this]() { setState(StateUpdateRequired); });

  connect(&m_private->m_controller, &Controller::readyToBackendFailure,
          [this]() {
            deleteTasks();
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

  connect(&m_gleanTimer, &QTimer::timeout, this, &MozillaVPN::sendGleanPings);
  m_gleanTimer.start(Constants::gleanTimeoutMsec());
  m_gleanTimer.setSingleShot(false);
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

void MozillaVPN::initialize() {
  logger.debug() << "MozillaVPN Initialization";

  Q_ASSERT(!m_initialized);
  m_initialized = true;

  // This is our first state.
  Q_ASSERT(m_state == StateInitialize);

  m_private->m_releaseMonitor.runSoon();

  scheduleTask(new TaskGetFeatureList());

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

#ifdef MVPN_IOS
  if (!settingsHolder->hasNativeIOSDataMigrated()) {
    IOSDataMigration::migrate();
    settingsHolder->setNativeIOSDataMigrated(true);
  }
#endif

#ifdef MVPN_WINDOWS
  if (!settingsHolder->hasNativeWindowsDataMigrated()) {
    WindowsDataMigration::migrate();
    settingsHolder->setNativeWindowsDataMigrated(true);
  }
#endif

#ifdef MVPN_ANDROID
  if (!settingsHolder->hasNativeAndroidDataMigrated()) {
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

  scheduleTask(new TaskAccountAndServers());

  scheduleTask(new TaskCaptivePortalLookup());

  if (FeatureInAppPurchase::instance()->isSupported()) {
    scheduleTask(new TaskProducts());
  }

  setUserAuthenticated(true);
  maybeStateMain();
}

void MozillaVPN::setState(State state) {
  logger.debug() << "Set state:" << QVariant::fromValue(state).toString();
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
  if (!settingsHolder->hasPostAuthenticationShown() ||
      !settingsHolder->postAuthenticationShown()) {
    setState(StatePostAuthentication);
    return;
  }
#endif

  if (!settingsHolder->hasTelemetryPolicyShown() ||
      !settingsHolder->telemetryPolicyShown()) {
    setState(StateTelemetryPolicy);
    return;
  }

  if (!m_private->m_deviceModel.hasCurrentDevice(keys())) {
    Q_ASSERT(m_private->m_deviceModel.activeDevices() ==
             m_private->m_user.maxDevices());
    emit triggerGleanSample(GleanSample::maxDeviceReached);
    setState(StateDeviceLimit);
    return;
  }

  if (m_state != StateUpdateRequired) {
    setState(StateMain);
  }
}

void MozillaVPN::setServerPublicKey(const QString& publicKey) {
  logger.debug() << "Set server public key:" << publicKey;
  m_serverPublicKey = publicKey;
}

void MozillaVPN::getStarted() {
  logger.debug() << "Get started";

  SettingsHolder* settingsHolder = SettingsHolder::instance();

  if (!settingsHolder->hasTelemetryPolicyShown() ||
      !settingsHolder->telemetryPolicyShown()) {
    setState(StateTelemetryPolicy);
    return;
  }

  authenticate();
}

void MozillaVPN::authenticate(
    MozillaVPN::AuthenticationType authenticationType) {
  logger.debug() << "Authenticate";

  setState(StateAuthenticating);

  hideAlert();

  if (authenticationType == DefaultAuthentication) {
    authenticationType = FeatureInAppAuth::instance()->isSupported()
                             ? AuthenticationInApp
                             : AuthenticationInBrowser;
  }

  if (m_userAuthenticated) {
    LogoutObserver* lo = new LogoutObserver(this);
    // Let's use QueuedConnection to avoid nexted tasks executions.
    connect(
        lo, &LogoutObserver::ready, this,
        [&] { authenticate(authenticationType); }, Qt::QueuedConnection);
    return;
  }

  emit triggerGleanSample(GleanSample::authenticationStarted);

  scheduleTask(new TaskHeartbeat());

  scheduleTask(new TaskAuthenticate(authenticationType));
}

void MozillaVPN::abortAuthentication() {
  logger.warning() << "Abort authentication";
  Q_ASSERT(m_state == StateAuthenticating);
  setState(StateInitialize);

  emit triggerGleanSample(GleanSample::authenticationAborted);
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

    case LinkLicense:
      url =
          "https://github.com/mozilla-mobile/mozilla-vpn-client/blob/main/"
          "LICENSE.md";
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

    default:
      qFatal("Unsupported link type!");
      return;
  }

  UrlOpener::open(url, addEmailAddress);
}

void MozillaVPN::scheduleTask(Task* task) {
  Q_ASSERT(task);
  logger.debug() << "Scheduling task: " << task->name();

  m_tasks.append(task);
  maybeRunTask();
}

void MozillaVPN::maybeRunTask() {
  logger.debug() << "Tasks: " << m_tasks.size();

  if (m_running_task || m_tasks.empty()) {
    return;
  }

  m_running_task = m_tasks.takeFirst();
  Q_ASSERT(m_running_task);

  QObject::connect(m_running_task, &Task::completed, this,
                   &MozillaVPN::taskCompleted);

  m_running_task->run(this);
}

void MozillaVPN::taskCompleted() {
  Q_ASSERT(m_running_task);

  logger.debug() << "Task completed:" << m_running_task->name();
  m_running_task->deleteLater();
  m_running_task->disconnect();
  m_running_task = nullptr;

  maybeRunTask();
}

void MozillaVPN::setToken(const QString& token) {
  SettingsHolder::instance()->setToken(token);
}

void MozillaVPN::authenticationCompleted(const QByteArray& json,
                                         const QString& token) {
  logger.debug() << "Authentication completed";

  emit triggerGleanSample(GleanSample::authenticationCompleted);

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
  setUserAuthenticated(true);

  if (FeatureInAppPurchase::instance()->isSupported()) {
    if (m_private->m_user.subscriptionNeeded()) {
      scheduleTask(new TaskProducts());
      scheduleTask(new TaskFunction([this](MozillaVPN*) { maybeStateMain(); }));
      return;
    }
  }

  completeActivation();
}

void MozillaVPN::completeActivation() {
  int deviceCount = m_private->m_deviceModel.activeDevices();
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
    scheduleTask(new TaskAccountAndServers());
  }

  if (FeatureInAppPurchase::instance()->isSupported()) {
    scheduleTask(new TaskProducts());
  }

  // Finally we are able to activate the client.
  scheduleTask(new TaskFunction([this](MozillaVPN*) {
    if (!modelsInitialized()) {
      logger.error() << "Failed to complete the authentication";
      errorHandle(ErrorHandler::RemoteServiceError);
      setUserAuthenticated(false);
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

  SettingsHolder::instance()->setPrivateKey(privateKey);
  SettingsHolder::instance()->setPublicKey(publicKey);
  m_private->m_keys.storeKeys(privateKey, publicKey);
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

void MozillaVPN::removeDeviceFromPublicKey(const QString& publicKey) {
  logger.debug() << "Remove device";

  const Device* device =
      m_private->m_deviceModel.deviceFromPublicKey(publicKey);
  if (device) {
    // Let's inform the UI about what is going to happen.
    emit deviceRemoving(publicKey);
    scheduleTask(new TaskRemoveDevice(publicKey));
  }

  if (m_state != StateDeviceLimit) {
    return;
  }

  // Let's recover from the device-limit mode.
  Q_ASSERT(!m_private->m_deviceModel.hasCurrentDevice(keys()));

  // Here we add the current device.
  addCurrentDeviceAndRefreshData();

  // Finally we are able to activate the client.
  scheduleTask(new TaskFunction([this](MozillaVPN*) {
    if (m_state != StateDeviceLimit) {
      return;
    }

    if (!modelsInitialized()) {
      logger.warning() << "Models not initialized yet";
      errorHandle(ErrorHandler::RemoteServiceError);
      SettingsHolder::instance()->clear();
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

  serializeLogs(out, [this, out, buffer, feedbackText, rating, category] {
    Q_ASSERT(out);
    Q_ASSERT(buffer);

    // buffer is getting copied by TaskSendFeedback so we can delete it
    // afterwards
    scheduleTask(new TaskSendFeedback(feedbackText, *buffer, rating, category));

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

  serializeLogs(out, [this, out, buffer, email, subject, issueText, category] {
    Q_ASSERT(out);
    Q_ASSERT(buffer);

    // buffer is getting copied by TaskCreateSupportTicket so we can delete it
    // afterwards
    scheduleTask(new TaskCreateSupportTicket(email, subject, issueText, *buffer,
                                             category));

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

  emit triggerGleanSample(GleanSample::authenticationAborted);

  reset(true);
}

void MozillaVPN::logout() {
  logger.debug() << "Logout";

  setAlert(LogoutAlert);

  deleteTasks();

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
    scheduleTask(new TaskRemoveDevice(keys()->publicKey()));
  }

  scheduleTask(new TaskFunction([](MozillaVPN* vpn) { vpn->reset(false); }));
}

void MozillaVPN::reset(bool forceInitialState) {
  logger.debug() << "Cleaning up all";

  deleteTasks();

  SettingsHolder::instance()->clear();
  m_private->m_keys.forgetKeys();
  m_private->m_serverData.forget();

  if (FeatureInAppPurchase::instance()->isSupported()) {
    IAPHandler* iap = IAPHandler::instance();
    iap->stopSubscription();
    iap->stopProductsRegistration();
  }

  setUserAuthenticated(false);

  if (forceInitialState) {
    setState(StateInitialize);
  }
}

void MozillaVPN::deleteTasks() {
  for (Task* task : m_tasks) {
    task->deleteLater();
  }

  m_tasks.clear();

  if (m_running_task) {
    m_running_task->deleteLater();
    m_running_task->disconnect();
    m_running_task = nullptr;
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
      // This type of error might be caused by switching the VPN
      // on, in which case it's okay to be ignored.
      // In Case the vpn is not connected - handle this like a
      // ConnectionFailureError
      if (controller()->state() == Controller::StateOn) {
        logger.error() << "Ignore network error probably caused by enabled VPN";
        break;
      }
      [[fallthrough]];
    case ErrorHandler::ConnectionFailureError:
      alert = ConnectionFailedAlert;
      break;

    case ErrorHandler::NoConnectionError:
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
      emit triggerGleanSample(GleanSample::authenticationFailureByGeo);
    } else {
      emit triggerGleanSample(GleanSample::authenticationFailure);
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

const Server& MozillaVPN::randomHop(ServerData& data) const {
  m_private->m_serverCountryModel.pickRandom(data);
  const QList<Server> servers = m_private->m_serverCountryModel.servers(data);
  return Server::weightChooser(servers);
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

void MozillaVPN::telemetryPolicyCompleted() {
  logger.debug() << "telemetry policy completed";

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  settingsHolder->setTelemetryPolicyShown(true);

  // Super racy, but it could happen that we are already in update-required
  // state.
  if (m_state == StateUpdateRequired) {
    return;
  }

  if (!m_userAuthenticated) {
    authenticate();
    return;
  }

  maybeStateMain();
}

void MozillaVPN::setUpdateRecommended(bool value) {
  m_updateRecommended = value;
  emit updateRecommendedChanged();
}

void MozillaVPN::setUserAuthenticated(bool state) {
  logger.debug() << "User authentication state:"
                 << QVariant::fromValue(state).toString();
  m_userAuthenticated = state;
  emit userAuthenticationChanged();
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

void MozillaVPN::viewLogs() {
  logger.debug() << "View logs";

  if (writeAndShowLogs(QStandardPaths::DesktopLocation)) {
    return;
  }

  if (writeAndShowLogs(QStandardPaths::HomeLocation)) {
    return;
  }

  if (writeAndShowLogs(QStandardPaths::TempLocation)) {
    return;
  }

  qWarning()
      << "No Desktop, no Home, no Temp folder. Unable to store the log files.";
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

  deleteTasks();
  scheduleTask(new TaskControllerAction(TaskControllerAction::eActivate));
}

void MozillaVPN::deactivate() {
  logger.debug() << "VPN tunnel deactivation";

  deleteTasks();
  scheduleTask(new TaskControllerAction(TaskControllerAction::eDeactivate));
}

void MozillaVPN::silentSwitch() {
  logger.debug() << "VPN tunnel silent server switch";

  scheduleTask(new TaskControllerAction(TaskControllerAction::eSilentSwitch));
}

void MozillaVPN::refreshDevices() {
  logger.debug() << "Refresh devices";

  if (m_state == StateMain) {
    scheduleTask(new TaskAccountAndServers());
  }
}

void MozillaVPN::quit() {
  logger.debug() << "quit";
  deleteTasks();
  qApp->quit();
}

void MozillaVPN::subscriptionStarted(const QString& productIdentifier) {
  logger.debug() << "Subscription started" << productIdentifier;

  setState(StateSubscriptionInProgress);

  IAPHandler* iap = IAPHandler::instance();

  // If IAP is not ready (race condition), register the products again.
  if (!iap->hasProductsRegistered()) {
    scheduleTask(new TaskProducts());
    scheduleTask(new TaskFunction([productIdentifier](MozillaVPN* vpn) {
      vpn->subscriptionStarted(productIdentifier);
    }));

    return;
  }

  iap->startSubscription(productIdentifier);
}

void MozillaVPN::subscriptionCompleted() {
  if (m_state != StateSubscriptionInProgress) {
    // We could hit this in android flow if we're doing a late acknowledgement.
    // And ignoring is fine.
    logger.warning()
        << "Random subscription completion received. Let's ignore it.";
    return;
  }

  logger.debug() << "Subscription completed";
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

  scheduleTask(new TaskAccountAndServers());
  scheduleTask(new TaskFunction([this](MozillaVPN*) {
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

  if (!modelsInitialized() || !m_userAuthenticated) {
    setState(StateInitialize);
    return;
  }

  maybeStateMain();
}

void MozillaVPN::triggerHeartbeat() { scheduleTask(new TaskHeartbeat()); }

void MozillaVPN::addCurrentDeviceAndRefreshData() {
  scheduleTask(new TaskAddDevice(Device::currentDeviceName()));
  scheduleTask(new TaskAccountAndServers());
}

void MozillaVPN::appReviewRequested() {
  Q_ASSERT(FeatureAppReview::instance()->isSupported());
#if defined(MVPN_IOS)
  IOSUtils::appReviewRequested();
#elif defined(MVPN_ANDROID)
  AndroidUtils::appReviewRequested();
#endif
}

bool MozillaVPN::validateUserDNS(const QString& dns) const {
  return DNSHelper::validateUserDNS(dns);
}
