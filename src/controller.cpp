/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "controller.h"
#include "captiveportal/captiveportal.h"
#include "captiveportal/captiveportalactivator.h"
#include "controllerimpl.h"
#include "ipaddressrange.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/server.h"
#include "mozillavpn.h"
#include "rfc1918.h"
#include "rfc4193.h"
#include "settingsholder.h"
#include "timercontroller.h"
#include "timersingleshot.h"

#if defined(MVPN_LINUX)
#  include "platforms/linux/linuxcontroller.h"
#elif defined(MVPN_MACOS_DAEMON) || defined(MVPN_WINDOWS)
#  include "localsocketcontroller.h"
#elif defined(MVPN_IOS) || defined(MVPN_MACOS_NETWORKEXTENSION)
#  include "platforms/ios/ioscontroller.h"
#elif defined(MVPN_ANDROID)
#  include "platforms/android/androidcontroller.h"
#else
#  include "platforms/dummy/dummycontroller.h"
#endif

constexpr const uint32_t TIMER_MSEC = 1000;

// X connection retries.
constexpr const int CONNECTION_MAX_RETRY = 9;

namespace {
Logger logger(LOG_CONTROLLER, "Controller");

ControllerImpl::Reason stateToReason(Controller::State state) {
  if (state == Controller::StateSwitching) {
    return ControllerImpl::ReasonSwitching;
  }

  if (state == Controller::StateConfirming) {
    return ControllerImpl::ReasonConfirming;
  }

  return ControllerImpl::ReasonNone;
}

}  // namespace

Controller::Controller() {
  MVPN_COUNT_CTOR(Controller);

  connect(&m_timer, &QTimer::timeout, this, &Controller::timerTimeout);

  connect(&m_connectionCheck, &ConnectionCheck::success, this,
          &Controller::connectionConfirmed);
  connect(&m_connectionCheck, &ConnectionCheck::failure, this,
          &Controller::connectionFailed);
}

Controller::~Controller() { MVPN_COUNT_DTOR(Controller); }

Controller::State Controller::state() const { return m_state; }

void Controller::initialize() {
  logger.log() << "Initializing the controller";

  if (m_state != StateInitializing) {
    setState(StateInitializing);
  }

  // Let's delete the previous controller before creating a new one.
  if (m_impl) {
    m_impl.reset(nullptr);
  }

  m_impl.reset(new TimerController(
#if defined(MVPN_LINUX)
      new LinuxController()
#elif defined(MVPN_MACOS_DAEMON) || defined(MVPN_WINDOWS)
      new LocalSocketController()
#elif defined(MVPN_IOS) || defined(MVPN_MACOS_NETWORKEXTENSION)
      new IOSController()
#elif defined(MVPN_ANDROID)
      new AndroidController()
#else
      new DummyController()
#endif
          ));

  connect(m_impl.get(), &ControllerImpl::connected, this,
          &Controller::connected);
  connect(m_impl.get(), &ControllerImpl::disconnected, this,
          &Controller::disconnected);
  connect(m_impl.get(), &ControllerImpl::initialized, this,
          &Controller::implInitialized);
  connect(m_impl.get(), &ControllerImpl::statusUpdated, this,
          &Controller::statusUpdated);

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  const Device* device = vpn->deviceModel()->currentDevice(vpn->keys());
  m_impl->initialize(device, vpn->keys());
}

void Controller::implInitialized(bool status, bool a_connected,
                                 const QDateTime& connectionDate) {
  logger.log() << "Controller activated with status:" << status
               << "connected:" << a_connected
               << "connectionDate:" << connectionDate.toString();

  Q_ASSERT(m_state == StateInitializing);

  if (!status) {
    MozillaVPN::instance()->errorHandle(ErrorHandler::BackendServiceError);
  }

  Q_UNUSED(status);

  if (processNextStep()) {
    return;
  }

  setState(a_connected ? StateOn : StateOff);

  // If we are connected already at startup time, we can trigger the connection
  // sequence of tasks.
  if (a_connected) {
    m_connectionDate = connectionDate;
    emit timeChanged();
    m_timer.start(TIMER_MSEC);
    return;
  }

  if (SettingsHolder::instance()->startAtBoot()) {
    logger.log() << "Start on boot";
    activate();
  }
}

void Controller::activate() {
  logger.log() << "Activation" << m_state;

  if (m_state != StateOff && m_state != StateSwitching &&
      m_state != StateCaptivePortal) {
    logger.log() << "Already connected";
    return;
  }

  if (m_state == StateOff) {
    setState(StateConnecting);
  }

  m_timer.stop();
  resetConnectionCheck();

  activateInternal();
}

void Controller::activateInternal() {
  logger.log() << "Activation internal";

  m_connectionDate = QDateTime::currentDateTime();

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  QList<Server> servers = vpn->servers();
  Q_ASSERT(!servers.isEmpty());

  Server server = Server::weightChooser(servers);
  Q_ASSERT(server.initialized());

  const Device* device = vpn->deviceModel()->currentDevice(vpn->keys());

  const QList<IPAddressRange> allowedIPAddressRanges =
      getAllowedIPAddressRanges(server);

  QList<QString> vpnDisabledApps;

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  if (settingsHolder->protectSelectedApps() &&
      settingsHolder->hasVpnDisabledApps()) {
    vpnDisabledApps = settingsHolder->vpnDisabledApps();
  }

  Q_ASSERT(m_impl);
  m_impl->activate(server, device, vpn->keys(), allowedIPAddressRanges,
                   vpnDisabledApps, stateToReason(m_state));
}

void Controller::deactivate() {
  logger.log() << "Deactivation" << m_state;

  if (m_state != StateOn && m_state != StateSwitching &&
      m_state != StateConfirming) {
    logger.log() << "Already disconnected";
    return;
  }

  if (m_state == StateOn || m_state == StateConfirming) {
    setState(StateDisconnecting);
  }

  m_timer.stop();
  resetConnectionCheck();

  Q_ASSERT(m_impl);
  m_impl->deactivate(stateToReason(m_state));
}

void Controller::connected() {
  logger.log() << "Connected from state:" << m_state;

  // This is an unexpected connection. Let's use the Connecting state to animate
  // the UI.
  if (m_state != StateConnecting && m_state != StateSwitching &&
      m_state != StateConfirming) {
    setState(StateConnecting);

    m_connectionDate = QDateTime::currentDateTime();

    TimerSingleShot::create(this, TIME_ACTIVATION, [this]() {
      if (m_state == StateConnecting) {
        connected();
      }
    });
    return;
  }

  setState(StateConfirming);

  // Now, let's wait for a ping sent and received from ConnectionHealth.
  m_connectionCheck.start();
}

void Controller::connectionConfirmed() {
  logger.log() << "Connection confirmed";

  if (m_state != StateConfirming) {
    logger.log() << "Invalid confirmation received";
    return;
  }

  m_connectionRetry = 0;
  emit connectionRetryChanged();

  setState(StateOn);
  emit timeChanged();

  if (m_nextStep != None) {
    disconnect();
    return;
  }

  m_timer.start(TIMER_MSEC);
}

void Controller::connectionFailed() {
  logger.log() << "Connection failed!";

  if (m_state != StateConfirming) {
    logger.log() << "Invalid confirmation received";
    return;
  }

  if (m_connectionRetry >= CONNECTION_MAX_RETRY) {
    deactivate();
    return;
  }

  ++m_connectionRetry;
  emit connectionRetryChanged();

  m_expectDisconnection = true;

  Q_ASSERT(m_impl);
  m_impl->deactivate(ControllerImpl::ReasonConfirming);
}

void Controller::disconnected() {
  logger.log() << "Disconnected from state:" << m_state;

  if (m_expectDisconnection) {
    Q_ASSERT(m_state == StateConfirming);
    Q_ASSERT(m_connectionRetry > 0);

    m_expectDisconnection = false;

    // We are retrying the connection. Let's ignore this disconnect signal and
    // let's retrigger the connection.
    activateInternal();
    return;
  }

  m_timer.stop();
  resetConnectionCheck();

  // This is an unexpected disconnection. Let's use the Disconnecting state to
  // animate the UI.
  if (m_state != StateDisconnecting && m_state != StateSwitching) {
    setState(StateDisconnecting);
    TimerSingleShot::create(this, TIME_DEACTIVATION, [this]() {
      if (m_state == StateDisconnecting) {
        disconnected();
      }
    });
    return;
  }

  NextStep nextStep = m_nextStep;

  if (processNextStep()) {
    return;
  }

  if (nextStep == None && m_state == StateSwitching) {
    MozillaVPN::instance()->changeServer(m_switchingCountryCode,
                                         m_switchingCity);
    activate();
    return;
  }

  setState(StateOff);
}

void Controller::timerTimeout() {
  Q_ASSERT(m_state == StateOn);
  emit timeChanged();
}

void Controller::changeServer(const QString& countryCode, const QString& city) {
  Q_ASSERT(m_state == StateOn || m_state == StateOff);

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  if (vpn->currentServer()->countryCode() == countryCode &&
      vpn->currentServer()->city() == city) {
    logger.log() << "No server change needed";
    return;
  }

  if (m_state == StateOff) {
    logger.log() << "Change server";
    vpn->changeServer(countryCode, city);
    return;
  }

  m_timer.stop();
  resetConnectionCheck();

  logger.log() << "Switching to a different server";

  m_currentCity = vpn->currentServer()->city();
  m_switchingCountryCode = countryCode;
  m_switchingCity = city;

  setState(StateSwitching);

  deactivate();
}

void Controller::quit() {
  logger.log() << "Quitting";

  if (m_state == StateInitializing || m_state == StateOff ||
      m_state == StateCaptivePortal) {
    emit readyToQuit();
    return;
  }

  m_nextStep = Quit;

  if (m_state == StateOn) {
    deactivate();
    return;
  }
}

void Controller::updateRequired() {
  logger.log() << "Update required";

  if (m_state == StateOff) {
    emit readyToUpdate();
    return;
  }

  m_nextStep = Update;

  if (m_state == StateOn) {
    deactivate();
    return;
  }
}

void Controller::logout() {
  logger.log() << "Logout";

  MozillaVPN::instance()->logout();

  if (m_state == StateOff) {
    return;
  }

  m_nextStep = Disconnect;

  if (m_state == StateOn) {
    deactivate();
    return;
  }
}

bool Controller::processNextStep() {
  NextStep nextStep = m_nextStep;
  m_nextStep = None;

  if (nextStep == Quit) {
    emit readyToQuit();
    return true;
  }

  if (nextStep == Update) {
    emit readyToUpdate();
    return true;
  }

  if (nextStep == WaitForCaptivePortal) {
    CaptivePortalActivator* activator = new CaptivePortalActivator(this);
    activator->run();

    setState(StateCaptivePortal);
    return true;
  }

  return false;
}

void Controller::setState(State state) {
  logger.log() << "Setting state:" << state;

  if (m_state != state) {
    m_state = state;
    emit stateChanged();
  }
}

int Controller::time() const {
  return (int)(m_connectionDate.msecsTo(QDateTime::currentDateTime()) / 1000);
}

void Controller::getBackendLogs(
    std::function<void(const QString&)>&& a_callback) {
  std::function<void(const QString&)> callback = std::move(a_callback);

  if (!m_impl) {
    callback(QString());
    return;
  }

  m_impl->getBackendLogs(std::move(callback));
}

void Controller::cleanupBackendLogs() {
  if (m_impl) {
    m_impl->cleanupBackendLogs();
  }
}

void Controller::getStatus(
    std::function<void(const QString& serverIpv4Gateway, uint64_t txByte,
                       uint64_t rxBytes)>&& a_callback) {
  logger.log() << "check status";

  std::function<void(const QString& serverIpv4Gateway, uint64_t txBytes,
                     uint64_t rxBytes)>
      callback = std::move(a_callback);

  if (m_state != StateOn && m_state != StateConfirming) {
    callback(QString(), 0, 0);
    return;
  }

  bool requestStatus = m_getStatusCallbacks.isEmpty();

  m_getStatusCallbacks.append(std::move(callback));

  if (m_impl && requestStatus) {
    m_impl->checkStatus();
  }
}

void Controller::statusUpdated(const QString& serverIpv4Gateway,
                               uint64_t txBytes, uint64_t rxBytes) {
  logger.log() << "Status updated";
  QList<std::function<void(const QString& serverIpv4Gateway, uint64_t txBytes,
                           uint64_t rxBytes)>>
      list;

  list.swap(m_getStatusCallbacks);
  for (const std::function<void(const QString&serverIpv4Gateway,
                                uint64_t txBytes, uint64_t rxBytes)>&func :
       list) {
    func(serverIpv4Gateway, txBytes, rxBytes);
  }
}

void Controller::captivePortalDetected() {
  logger.log() << "Captive portal detected in state:" << m_state;

  if (m_state != StateOn && m_state != StateConfirming) {
    return;
  }

  m_nextStep = WaitForCaptivePortal;
  deactivate();
}

QList<IPAddressRange> Controller::getAllowedIPAddressRanges(
    const Server& server) {
  bool ipv6Enabled = SettingsHolder::instance()->ipv6Enabled();

  QList<IPAddressRange> list;
#if 0
    if (SettingsHolder::instance()->captivePortalAlert()) {
        CaptivePortal *captivePortal = MozillaVPN::instance()->captivePortal();
        const QStringList &captivePortalIpv4Addresses = captivePortal->ipv4Addresses();
        for (const QString &address : captivePortalIpv4Addresses) {
            list.append(IPAddressRange(address, 0, IPAddressRange::IPv4));
        }

        if (ipv6Enabled) {
            const QStringList &captivePortalIpv6Addresses = captivePortal->ipv6Addresses();
            for (const QString &address : captivePortalIpv6Addresses) {
                list.append(IPAddressRange(address, 0, IPAddressRange::IPv6));
            }
        }
    }
#endif

  if (MozillaVPN::instance()->localNetworkAccessSupported() &&
      SettingsHolder::instance()->localNetworkAccess()) {
    // In case of lan enabled, whitelist all non LAN ip's
    list.append(RFC1918::ipv4());
    if (ipv6Enabled) {
      list.append(RFC4193::ipv6());
    }
    // Whitelist the servers gateway -
    // otherwise we can't ping it for connectionhealth
    list.append(IPAddressRange(server.ipv4Gateway(), 32, IPAddressRange::IPv4));

  } else {
    // Add catchall-range in case LAN is disabled
    list.append(IPAddressRange("0.0.0.0", 0, IPAddressRange::IPv4));
    if (ipv6Enabled) {
      list.append(IPAddressRange("::0", 0, IPAddressRange::IPv6));
    }
  }

  return list;
}

void Controller::resetConnectionCheck() {
  m_expectDisconnection = false;

  m_connectionCheck.stop();

  m_connectionRetry = 0;
  emit connectionRetryChanged();
}
