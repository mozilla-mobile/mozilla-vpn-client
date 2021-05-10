/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozillavpn.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"
#include "logoutobserver.h"
#include "models/device.h"
#include "models/servercountrymodel.h"
#include "models/user.h"
#include "qmlengineholder.h"
#include "settingsholder.h"
#include "tasks/accountandservers/taskaccountandservers.h"
#include "tasks/adddevice/taskadddevice.h"
#include "tasks/authenticate/taskauthenticate.h"
#include "tasks/captiveportallookup/taskcaptiveportallookup.h"
#include "tasks/controlleraction/taskcontrolleraction.h"
#include "tasks/function/taskfunction.h"
#include "tasks/heartbeat/taskheartbeat.h"
#include "tasks/removedevice/taskremovedevice.h"
#include "urlopener.h"

#ifdef MVPN_IOS
#  include "platforms/ios/iaphandler.h"
#  include "platforms/ios/iosdatamigration.h"
#  include "platforms/ios/taskiosproducts.h"
#endif

#ifdef MVPN_ANDROID
#  include "platforms/android/androidutils.h"
#endif

#ifdef MVPN_WINDOWS
#  include "platforms/windows/windowsdatamigration.h"
#endif
#ifdef MVPN_ANDROID
#  include "platforms/android/androiddatamigration.h"
#  include "platforms/android/androidvpnactivity.h"
#endif

#ifdef MVPN_INSPECTOR
#  include "inspector/inspectorwebsocketconnection.h"
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

  logger.log() << "Creating MozillaVPN singleton";

  Q_ASSERT(!s_instance);
  s_instance = this;

  connect(&m_alertTimer, &QTimer::timeout, [this]() { setAlert(NoAlert); });

  connect(&m_periodicOperationsTimer, &QTimer::timeout, [this]() {
    scheduleTask(new TaskAccountAndServers());
    scheduleTask(new TaskCaptivePortalLookup());
    scheduleTask(new TaskHeartbeat());
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

#ifdef MVPN_IOS
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
#endif
}

MozillaVPN::~MozillaVPN() {
  MVPN_COUNT_DTOR(MozillaVPN);

  logger.log() << "Deleting MozillaVPN singleton";

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;

  delete m_private;
}

MozillaVPN::State MozillaVPN::state() const { return m_state; }

void MozillaVPN::initialize() {
  logger.log() << "MozillaVPN Initialization";

  Q_ASSERT(!m_initialized);
  m_initialized = true;

  // This is our first state.
  Q_ASSERT(m_state == StateInitialize);

  m_private->m_releaseMonitor.runSoon();

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

  logger.log() << "We have a valid token";
  if (!m_private->m_user.fromSettings()) {
    return;
  }

  if (!m_private->m_keys.fromSettings()) {
    logger.log() << "No keys found";
    settingsHolder->clear();
    return;
  }

  if (!m_private->m_serverCountryModel.fromSettings()) {
    logger.log() << "No server list found";
    settingsHolder->clear();
    return;
  }

  if (!m_private->m_deviceModel.fromSettings(keys())) {
    logger.log() << "No devices found";
    settingsHolder->clear();
    return;
  }

  if (!m_private->m_deviceModel.hasCurrentDevice(keys())) {
    logger.log() << "The current device has not been found";
    settingsHolder->clear();
    return;
  }

  if (!m_private->m_captivePortal.fromSettings()) {
    // We do not care about CaptivePortal settings.
  }

  if (!modelsInitialized()) {
    logger.log() << "Models not initialized yet";
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

#ifdef MVPN_IOS
  scheduleTask(new TaskIOSProducts());
#endif

  setUserAuthenticated(true);
  maybeStateMain();
}

void MozillaVPN::setState(State state) {
  logger.log() << "Set state:" << state;
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
  logger.log() << "Maybe state main";

#ifdef MVPN_IOS
  if (m_private->m_user.subscriptionNeeded()) {
    setState(StateSubscriptionNeeded);
    return;
  }
#endif

#if !defined(MVPN_ANDROID) && !defined(MVPN_IOS)
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  if (!settingsHolder->hasPostAuthenticationShown() ||
      !settingsHolder->postAuthenticationShown()) {
    settingsHolder->setPostAuthenticationShown(true);
    setState(StatePostAuthentication);
    return;
  }
#endif

  if (!m_private->m_deviceModel.hasCurrentDevice(keys())) {
    Q_ASSERT(m_private->m_deviceModel.activeDevices() ==
             m_private->m_user.maxDevices());
    setState(StateDeviceLimit);
    return;
  }

  if (m_state != StateUpdateRequired) {
    setState(StateMain);
  }
}

void MozillaVPN::setServerPublicKey(QString publicKey) {
  logger.log() << "Set server public key:" << publicKey;
  m_serverPublicKey = publicKey;
}

void MozillaVPN::authenticate() {
  logger.log() << "Authenticate";

  setState(StateAuthenticating);

  hideAlert();

  if (m_userAuthenticated) {
    LogoutObserver* lo = new LogoutObserver(this);
    // Let's use QueuedConnection to avoid nexted tasks executions.
    connect(lo, &LogoutObserver::ready, this, &MozillaVPN::authenticate,
            Qt::QueuedConnection);
    return;
  }

  scheduleTask(new TaskHeartbeat());
  scheduleTask(new TaskAuthenticate());
}

void MozillaVPN::abortAuthentication() {
  logger.log() << "Abort authentication";
  Q_ASSERT(m_state == StateAuthenticating);
  setState(StateInitialize);
}

void MozillaVPN::openLink(LinkType linkType) {
  logger.log() << "Opening link: " << linkType;

  QString url;

  switch (linkType) {
    case LinkAccount:
      url = Constants::API_URL;
      url.append("/r/vpn/account");
      break;

    case LinkContact:
      url = Constants::API_URL;
      url.append("/r/vpn/contact");
      break;

    case LinkFeedback:
      url = Constants::API_URL;
      url.append("/r/vpn/client/feedback");
      break;

    case LinkHelpSupport:
      url = Constants::API_URL;
      url.append("/r/vpn/support");
      break;

    case LinkLicense:
      url =
          "https://github.com/mozilla-mobile/mozilla-vpn-client/blob/main/"
          "LICENSE.md";
      break;

    case LinkTermsOfService:
      url = Constants::API_URL;
      url.append("/r/vpn/terms");
      break;

    case LinkPrivacyNotice:
      url = Constants::API_URL;
      url.append("/r/vpn/privacy");
      break;

    case LinkUpdate:
      url = Constants::API_URL;
      url.append("/r/vpn/update/");
#if defined(MVPN_LINUX)
      url.append("linux");
#elif defined(MVPN_MACOS)
      url.append("macos");
#elif defined(MVPN_IOS)
      url.append("ios");
#elif defined(MVPN_ANDROID)
      url.append("android");
#else
      url.append("dummy");
#endif
      break;

    case LinkSubscriptionBlocked:
      url = Constants::API_URL;
      url.append("/r/vpn/subscriptionBlocked");
      break;

    default:
      qFatal("Unsupported link type!");
      return;
  }

  UrlOpener::open(url);
}

void MozillaVPN::scheduleTask(Task* task) {
  Q_ASSERT(task);
  logger.log() << "Scheduling task: " << task->name();

  m_tasks.append(task);
  maybeRunTask();
}

void MozillaVPN::maybeRunTask() {
  logger.log() << "Tasks: " << m_tasks.size();

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
  logger.log() << "Task completed";

  Q_ASSERT(m_running_task);
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
  logger.log() << "Authentication completed";

  if (!m_private->m_user.fromJson(json)) {
    logger.log() << "Failed to parse the User JSON data";
    errorHandle(ErrorHandler::RemoteServiceError);
    return;
  }

  if (!m_private->m_deviceModel.fromJson(keys(), json)) {
    logger.log() << "Failed to parse the DeviceModel JSON data";
    errorHandle(ErrorHandler::RemoteServiceError);
    return;
  }

  m_private->m_user.writeSettings();
  m_private->m_deviceModel.writeSettings();

  setToken(token);
  setUserAuthenticated(true);

#ifdef MVPN_IOS
  if (m_private->m_user.subscriptionNeeded()) {
    scheduleTask(new TaskIOSProducts());
    scheduleTask(new TaskFunction([this](MozillaVPN*) { maybeStateMain(); }));
    return;
  }
#endif

  completeActivation();
}

MozillaVPN::RemovalDeviceOption MozillaVPN::maybeRemoveCurrentDevice() {
  logger.log() << "Maybe remove current device";

  const Device* currentDevice =
      m_private->m_deviceModel.device(Device::currentDeviceName());
  if (!currentDevice) {
    logger.log() << "No removal needed because the device doesn't exist yet";
    return DeviceNotFound;
  }

  if (currentDevice->publicKey() == m_private->m_keys.publicKey() &&
      !m_private->m_keys.privateKey().isEmpty()) {
    logger.log() << "No removal needed because the private key is still fine.";
    return DeviceStillValid;
  }

  logger.log() << "Removal needed";
  scheduleTask(new TaskRemoveDevice(currentDevice->publicKey()));
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
  if (option != DeviceStillValid) {
    scheduleTask(new TaskAddDevice(Device::currentDeviceName()));
  }

  // Let's fetch the account and the servers.
  scheduleTask(new TaskAccountAndServers());

#ifdef MVPN_IOS
  scheduleTask(new TaskIOSProducts());
#endif

  // Finally we are able to activate the client.
  scheduleTask(new TaskFunction([this](MozillaVPN*) {
    if (!modelsInitialized()) {
      logger.log() << "Failed to complete the authentication";
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
  logger.log() << "Device added" << deviceName;

  SettingsHolder::instance()->setPrivateKey(privateKey);
  SettingsHolder::instance()->setPublicKey(publicKey);
  m_private->m_keys.storeKeys(privateKey, publicKey);
}

void MozillaVPN::deviceRemoved(const QString& deviceName) {
  logger.log() << "Device removed";

  m_private->m_deviceModel.removeDevice(deviceName);
}

bool MozillaVPN::setServerList(const QByteArray& serverData) {
  if (!m_private->m_serverCountryModel.fromJson(serverData)) {
    logger.log() << "Failed to store the server-countries";
    return false;
  }

  SettingsHolder::instance()->setServers(serverData);
  return true;
}

void MozillaVPN::serversFetched(const QByteArray& serverData) {
  logger.log() << "Server fetched!";

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

void MozillaVPN::removeDevice(const QString& deviceName) {
  logger.log() << "Remove device" << deviceName;

  // Let's inform the UI about what is going to happen.
  emit deviceRemoving(deviceName);

  const Device* device = m_private->m_deviceModel.device(deviceName);
  if (device) {
    scheduleTask(new TaskRemoveDevice(device->publicKey()));
  }

  if (m_state != StateDeviceLimit) {
    return;
  }

  // Let's recover from the device-limit mode.
  Q_ASSERT(!m_private->m_deviceModel.hasCurrentDevice(keys()));

  // Here we add the current device.
  scheduleTask(new TaskAddDevice(Device::currentDeviceName()));

  // Let's fetch the devices again.
  scheduleTask(new TaskAccountAndServers());

  // Finally we are able to activate the client.
  scheduleTask(new TaskFunction([this](MozillaVPN*) {
    if (m_state != StateDeviceLimit) {
      return;
    }

    if (!modelsInitialized()) {
      logger.log() << "Models not initialized yet";
      errorHandle(ErrorHandler::RemoteServiceError);
      SettingsHolder::instance()->clear();
      setState(StateInitialize);
      return;
    }

    maybeStateMain();
  }));
}

void MozillaVPN::accountChecked(const QByteArray& json) {
  logger.log() << "Account checked";

  if (!m_private->m_user.fromJson(json)) {
    logger.log() << "Failed to parse the User JSON data";
    // We don't need to communicate it to the user. Let's ignore it.
    return;
  }

  if (!m_private->m_deviceModel.fromJson(keys(), json)) {
    logger.log() << "Failed to parse the DeviceModel JSON data";
    // We don't need to communicate it to the user. Let's ignore it.
    return;
  }

  m_private->m_user.writeSettings();
  m_private->m_deviceModel.writeSettings();

#ifdef MVPN_IOS
  if (m_private->m_user.subscriptionNeeded() && m_state == StateMain) {
    maybeStateMain();
    return;
  }
#endif

  // To test the subscription needed view, comment out this line:
  // m_private->m_controller.subscriptionNeeded();
}

void MozillaVPN::cancelAuthentication() {
  logger.log() << "Canceling authentication";

  if (m_state != StateAuthenticating) {
    // We cannot cancel tasks if we are not in authenticating state.
    return;
  }

  reset(true);
}

void MozillaVPN::logout() {
  logger.log() << "Logout";

  setAlert(LogoutAlert);

  deleteTasks();

#ifdef MVPN_IOS
  IAPHandler::instance()->stopSubscription();
#endif

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
  logger.log() << "Cleaning up all";

  deleteTasks();

  SettingsHolder::instance()->clear();
  m_private->m_keys.forgetKeys();
  m_private->m_serverData.forget();

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
  logger.log() << "Handling error" << error;

  Q_ASSERT(error != ErrorHandler::NoError);

  AlertType alert = NoAlert;

  switch (error) {
    case ErrorHandler::VPNDependentConnectionError:
      // This type of error might be caused by switchting the VPN
      // on, in which case it's okay to be ignored.
      // In Case the vpn is not connected - handle this like a
      // ConnectionFailureError
      if (controller()->state() == Controller::StateOn) {
        logger.log() << "Ignore network error probably caused by enabled VPN";
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
      if (m_userAuthenticated) {
        alert = AuthenticationFailedAlert;
      }
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

  logger.log() << "Alert:" << alert << "State:" << m_state;

  if (alert == NoAlert) {
    return;
  }

  // Any error in authenticating state sends to the Initial state.
  if (m_state == StateAuthenticating) {
    setState(StateInitialize);
    return;
  }

  if (alert == AuthenticationFailedAlert) {
    reset(true);
    return;
  }
}

const QList<Server> MozillaVPN::servers() const {
  return m_private->m_serverCountryModel.servers(m_private->m_serverData);
}

void MozillaVPN::changeServer(const QString& countryCode, const QString& city) {
  QString countryName =
      m_private->m_serverCountryModel.countryName(countryCode);

  m_private->m_serverData.update(countryCode, countryName, city);
  m_private->m_serverData.writeSettings();
}

void MozillaVPN::postAuthenticationCompleted() {
  logger.log() << "Post authentication completed";

  // Super racy, but it could happen that we are already in update-required
  // state.
  if (m_state == StateUpdateRequired) {
    return;
  }

  maybeStateMain();
}

void MozillaVPN::setUpdateRecommended(bool value) {
  m_updateRecommended = value;
  emit updateRecommendedChanged();
}

void MozillaVPN::setUserAuthenticated(bool state) {
  logger.log() << "User authentication state:" << state;
  m_userAuthenticated = state;
  emit userAuthenticationChanged();
}

void MozillaVPN::startSchedulingPeriodicOperations() {
  logger.log() << "Start scheduling account and servers"
               << Constants::SCHEDULE_ACCOUNT_AND_SERVERS_TIMER_MSEC;
  m_periodicOperationsTimer.start(
      Constants::SCHEDULE_ACCOUNT_AND_SERVERS_TIMER_MSEC);
}

void MozillaVPN::stopSchedulingPeriodicOperations() {
  logger.log() << "Stop scheduling account and servers";
  m_periodicOperationsTimer.stop();
}

bool MozillaVPN::writeAndShowLogs(QStandardPaths::StandardLocation location) {
  return writeLogs(location, [](const QString& filename) {
    logger.log() << "Opening the logFile somehow:" << filename;
    QUrl url = QUrl::fromLocalFile(filename);
    UrlOpener::open(url);
  });
}

bool MozillaVPN::writeLogs(
    QStandardPaths::StandardLocation location,
    std::function<void(const QString& filename)>&& a_callback) {
  logger.log() << "Trying to save logs in:" << location;

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
    logger.log() << logFile << "exists. Let's try a new filename";

    for (uint32_t i = 1;; ++i) {
      QString filename;
      QTextStream(&filename)
          << "mozillavpn-" << now.year() << "-" << now.month() << "-"
          << now.day() << "_" << i << ".txt";
      logFile = logDir.filePath(filename);
      if (!QFileInfo::exists(logFile)) {
        logger.log() << "Filename found!" << i;
        break;
      }
    }
  }

  logger.log() << "Writing logs into: " << logFile;

  QFile* file = new QFile(logFile);
  if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
    logger.log() << "Failed to open the logfile";
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
        logger.log() << "Logs from the backend service received";

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

        finalizeCallback();
      });
}

void MozillaVPN::viewLogs() {
  logger.log() << "View logs";

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
  logger.log() << "Retrieve logs";

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
  logger.log() << "Store in clipboard";
  QApplication::clipboard()->setText(text);
}

void MozillaVPN::cleanupLogs() {
  logger.log() << "Cleanup logs";
  LogHandler::instance()->cleanupLogs();
  MozillaVPN::instance()->controller()->cleanupBackendLogs();
}

bool MozillaVPN::modelsInitialized() const {
  logger.log() << "Checking model initialization";
  if (!m_private->m_user.initialized()) {
    logger.log() << "User model not initialized";
    return false;
  }

  if (!m_private->m_serverCountryModel.initialized()) {
    logger.log() << "Server country model not initialized";
    return false;
  }

  if (!m_private->m_deviceModel.initialized()) {
    logger.log() << "Device model not initialized";
    return false;
  }

  if (!m_private->m_deviceModel.hasCurrentDevice(&m_private->m_keys)) {
    logger.log() << "Current device not registered";
    return false;
  }

  if (!m_private->m_keys.initialized()) {
    logger.log() << "Key model not initialized";
    return false;
  }

  return true;
}

void MozillaVPN::requestSettings() {
  logger.log() << "Settings required";

  QmlEngineHolder::instance()->showWindow();
  emit settingsNeeded();
}

void MozillaVPN::requestAbout() {
  logger.log() << "About view requested";

  QmlEngineHolder::instance()->showWindow();
  emit aboutNeeded();
}

void MozillaVPN::requestViewLogs() {
  logger.log() << "View log requested";

  QmlEngineHolder::instance()->showWindow();
  emit viewLogsNeeded();
}

void MozillaVPN::activate() {
  logger.log() << "VPN tunnel activation";

  deleteTasks();
  scheduleTask(new TaskControllerAction(TaskControllerAction::eActivate));
}

void MozillaVPN::deactivate() {
  logger.log() << "VPN tunnel deactivation";

  deleteTasks();
  scheduleTask(new TaskControllerAction(TaskControllerAction::eDeactivate));
}

void MozillaVPN::refreshDevices() {
  logger.log() << "Refresh devices";

  if (m_state == StateMain) {
    scheduleTask(new TaskAccountAndServers());
  }
}

void MozillaVPN::quit() {
  logger.log() << "quit";
  deleteTasks();
  qApp->quit();
}

#ifdef MVPN_IOS
void MozillaVPN::subscriptionStarted() {
  logger.log() << "Subscription started";

  setState(StateSubscriptionValidation);

  IAPHandler* iap = IAPHandler::instance();

  // If IPA is not ready yet (race condition), let's register the products
  // again.
  if (!iap->hasProductsRegistered()) {
    scheduleTask(new TaskIOSProducts());
    scheduleTask(
        new TaskFunction([](MozillaVPN* vpn) { vpn->subscriptionStarted(); }));

    return;
  }

  iap->startSubscription();
}

void MozillaVPN::subscriptionCompleted() {
  if (m_state != StateSubscriptionValidation) {
    logger.log() << "Random subscription completion received. Let's ignore it.";
    return;
  }

  logger.log() << "Subscription completed";
  completeActivation();
}

void MozillaVPN::subscriptionFailed() {
  subscriptionFailedInternal(false /* canceled by user */);
}

void MozillaVPN::subscriptionCanceled() {
  subscriptionFailedInternal(true /* canceled by user */);
}

void MozillaVPN::subscriptionFailedInternal(bool canceledByUser) {
  if (m_state != StateSubscriptionValidation) {
    logger.log() << "Random subscription failure received. Let's ignore it.";
    return;
  }

  logger.log() << "Subscription failed or canceled";

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
  if (m_state != StateSubscriptionValidation) {
    logger.log()
        << "Random already-subscribed notification received. Let's ignore it.";
    return;
  }

  setState(StateSubscriptionBlocked);
}
#endif

void MozillaVPN::update() {
  logger.log() << "Update";

  setUpdating(true);

  if (m_private->m_controller.state() != Controller::StateOff &&
      m_private->m_controller.state() != Controller::StateInitializing) {
    deactivate();
    return;
  }

  m_private->m_releaseMonitor.update();
}

void MozillaVPN::setUpdating(bool updating) {
  m_updating = updating;
  emit updatingChanged();
}

void MozillaVPN::controllerStateChanged() {
  logger.log() << "Controller state changed";

  if (!m_controllerInitialized) {
    m_controllerInitialized = true;

    if (SettingsHolder::instance()->startAtBoot()) {
      logger.log() << "Start on boot";
      activate();
    }
  }

  if (m_updating && m_private->m_controller.state() == Controller::StateOff) {
    update();
  }

  NetworkManager::instance()->clearCache();
}

void MozillaVPN::backendServiceRestore() {
  logger.log() << "Background service restore request";
  // TODO
}

void MozillaVPN::heartbeatCompleted(bool success) {
  logger.log() << "Server-side check done:" << success;

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
