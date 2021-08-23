/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "controller.h"
#include "controllerimpl.h"
#include "featurelist.h"
#include "features/featurecustomdns.h"
#include "features/featurecaptiveportal.h"
#include "features/featurelocalareaaccess.h"
#include "features/featuremultihop.h"

#include "ipaddress.h"
#include "ipaddressrange.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/server.h"
#include "mozillavpn.h"
#include "rfc/rfc1918.h"
#include "rfc/rfc4193.h"
#include "rfc/rfc5735.h"
#include "serveri18n.h"
#include "settingsholder.h"
#include "tasks/heartbeat/taskheartbeat.h"
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
  logger.debug() << "Initializing the controller";

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
  logger.debug() << "Controller activated with status:" << status
                 << "connected:" << a_connected
                 << "connectionDate:" << connectionDate.toString();

  Q_ASSERT(m_state == StateInitializing);

  if (!status) {
    MozillaVPN::instance()->errorHandle(ErrorHandler::ControllerError);
    setState(StateOff);
    return;
  }

  if (processNextStep()) {
    setState(StateOff);
    return;
  }

  setState(a_connected ? StateOn : StateOff);

  // If we are connected already at startup time, we can trigger the connection
  // sequence of tasks.
  if (a_connected) {
    m_connectedTimeInUTC = connectionDate.toUTC();
    emit timeChanged();
    m_timer.start(TIMER_MSEC);
  }
}

bool Controller::activate() {
  logger.debug() << "Activation" << m_state;

  if (m_state != StateOff && m_state != StateSwitching) {
    logger.debug() << "Already connected";
    return false;
  }

  if (m_state == StateOff) {
    setState(StateConnecting);
  }

  m_timer.stop();
  resetConnectionCheck();

  activateInternal();
  return true;
}

void Controller::activateInternal() {
  logger.debug() << "Activation internal";

  resetConnectedTime();

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  QList<Server> servers = vpn->servers();
  Q_ASSERT(!servers.isEmpty());

  Server server = Server::weightChooser(servers);
  Q_ASSERT(server.initialized());

  vpn->setServerPublicKey(server.publicKey());

  const Device* device = vpn->deviceModel()->currentDevice(vpn->keys());

  QList<QString> vpnDisabledApps;

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  if (settingsHolder->protectSelectedApps() &&
      settingsHolder->hasVpnDisabledApps()) {
    vpnDisabledApps = settingsHolder->vpnDisabledApps();
  }

  // Multihop connections provide a list of servers, starting with the exit
  // node as the first element, and the entry node as the final entry.
  QList<Server> serverList = {server};
  if (FeatureMultiHop::instance()->isSupported() &&
      settingsHolder->multihopTunnel()) {
    ServerData data;
    Server hop = vpn->randomHop(data);
    while (serverList.contains(hop)) {
      hop = vpn->randomHop(data);
    }
    serverList.append(hop);
  }

  // Use the Gateway as DNS Server
  // If the user as entered a valid dns, use that instead
  QHostAddress dns = QHostAddress(server.ipv4Gateway());
  if (FeatureCustomDNS::instance()->isSupported() &&
      !settingsHolder->useGatewayDNS() &&
      settingsHolder->userDNS().size() > 0 &&
      settingsHolder->validateUserDNS(settingsHolder->userDNS())) {
    dns = QHostAddress(settingsHolder->userDNS());
    logger.debug() << "User DNS Set" << dns.toString();
  }

  Q_ASSERT(m_impl);
  m_impl->activate(serverList, device, vpn->keys(),
                   getAllowedIPAddressRanges(serverList), vpnDisabledApps, dns,
                   stateToReason(m_state));
}

bool Controller::silentSwitchServers() {
  logger.debug() << "Silently switch servers";

  if (m_state != StateOn) {
    logger.warning() << "Cannot silent switch if not on";
    return false;
  }

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  QList<Server> servers = vpn->servers();
  Q_ASSERT(!servers.isEmpty());

  if (servers.length() <= 1) {
    logger.warning()
        << "Cannot silent switch servers because there is only one available";
    return false;
  }

  QList<Server>::iterator iterator = servers.begin();

  while (iterator != servers.end()) {
    if (iterator->publicKey() == vpn->serverPublicKey()) {
      servers.erase(iterator);
      break;
    }
    ++iterator;
  }

  Server server = Server::weightChooser(servers);
  Q_ASSERT(server.initialized());

#ifndef MVPN_WASM
  // All the keys are the same in WASM builds.
  Q_ASSERT(server.publicKey() != vpn->serverPublicKey());
#endif

  vpn->setServerPublicKey(server.publicKey());

  const Device* device = vpn->deviceModel()->currentDevice(vpn->keys());

  QList<QString> vpnDisabledApps;

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  if (settingsHolder->protectSelectedApps() &&
      settingsHolder->hasVpnDisabledApps()) {
    vpnDisabledApps = settingsHolder->vpnDisabledApps();
  }

  QList<Server> serverList = {server};
  if (FeatureMultiHop::instance()->isSupported() &&
      settingsHolder->multihopTunnel()) {
    ServerData data;
    Server hop = vpn->randomHop(data);
    while (serverList.contains(hop)) {
      hop = vpn->randomHop(data);
    }
    serverList.append(hop);
  }

  QHostAddress dns = QHostAddress(server.ipv4Gateway());
  if (FeatureCustomDNS::instance()->isSupported() &&
      !settingsHolder->useGatewayDNS() &&
      settingsHolder->userDNS().size() > 0 &&
      settingsHolder->validateUserDNS(settingsHolder->userDNS())) {
    dns = QHostAddress(settingsHolder->userDNS());
    logger.debug() << "User DNS Set" << dns.toString();
  }

  Q_ASSERT(m_impl);
  m_impl->activate(serverList, device, vpn->keys(),
                   getAllowedIPAddressRanges(serverList), vpnDisabledApps, dns,
                   stateToReason(StateSwitching));
  return true;
}

bool Controller::deactivate() {
  logger.debug() << "Deactivation" << m_state;

  if (m_state != StateOn && m_state != StateSwitching &&
      m_state != StateConfirming) {
    logger.warning() << "Already disconnected";
    return false;
  }

  if (m_state == StateOn || m_state == StateConfirming) {
    setState(StateDisconnecting);
  }

  m_timer.stop();
  resetConnectionCheck();

  Q_ASSERT(m_impl);
  m_impl->deactivate(stateToReason(m_state));
  return true;
}

void Controller::connected() {
  logger.debug() << "Connected from state:" << m_state;

  // We are currently silently switching servers
  if (m_state == StateOn) {
    m_connectionCheck.start();
    return;
  }

  // This is an unexpected connection. Let's use the Connecting state to animate
  // the UI.
  if (m_state != StateConnecting && m_state != StateSwitching &&
      m_state != StateConfirming) {
    setState(StateConnecting);

    resetConnectedTime();

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
  logger.debug() << "Connection confirmed";

  if (m_state != StateConfirming && m_state != StateOn) {
    logger.error() << "Invalid confirmation received";
    return;
  }

  m_connectionRetry = 0;
  emit connectionRetryChanged();

  if (m_state == StateOn) {
    emit silentSwitchDone();
    return;
  }

  setState(StateOn);
  emit timeChanged();

  if (m_nextStep != None) {
    deactivate();
    return;
  }

  m_timer.start(TIMER_MSEC);
}

void Controller::connectionFailed() {
  logger.debug() << "Connection failed!";

  if (m_state != StateConfirming && m_state != StateOn) {
    logger.error() << "Invalid confirmation received";
    return;
  }

  if (m_state == StateOn) {
    emit silentSwitchDone();
  }

  if (m_nextStep != None || m_connectionRetry >= CONNECTION_MAX_RETRY) {
    deactivate();
    return;
  }

  ++m_connectionRetry;
  emit connectionRetryChanged();

  m_reconnectionStep = ExpectDisconnection;

  Q_ASSERT(m_impl);
  m_impl->deactivate(ControllerImpl::ReasonConfirming);
}

void Controller::disconnected() {
  logger.debug() << "Disconnected from state:" << m_state;

  if (m_reconnectionStep == ExpectDisconnection) {
    Q_ASSERT(m_state == StateConfirming || m_state == StateOn);
    Q_ASSERT(m_connectionRetry > 0);

    // We are retrying the connection. Let's ignore this disconnect signal and
    // let's see if the servers are up.

    m_reconnectionStep = ExpectHeartbeat;

    TaskHeartbeat* task = new TaskHeartbeat();
    connect(task, &Task::completed, this, &Controller::heartbeatCompleted);
    task->run(MozillaVPN::instance());
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
    setState(StateOff);
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
      vpn->currentServer()->cityName() == city) {
    logger.debug() << "No server change needed";
    return;
  }

  if (m_state == StateOff) {
    logger.debug() << "Change server";
    vpn->changeServer(countryCode, city);
    return;
  }

  m_timer.stop();
  resetConnectionCheck();

  logger.debug() << "Switching to a different server";

  m_currentCity = vpn->currentServer()->cityName();
  m_currentCountryCode = vpn->currentServer()->countryCode();
  m_switchingCountryCode = countryCode;
  m_switchingCity = city;

  setState(StateSwitching);

  deactivate();
}

void Controller::quit() {
  logger.debug() << "Quitting";

  if (m_state == StateInitializing || m_state == StateOff) {
    emit readyToQuit();
    return;
  }

  m_nextStep = Quit;

  if (m_state == StateOn) {
    deactivate();
    return;
  }
}

void Controller::backendFailure() {
  logger.error() << "backend failure";

  if (m_state == StateInitializing || m_state == StateOff) {
    emit readyToBackendFailure();
    return;
  }

  m_nextStep = BackendFailure;

  if (m_state == StateOn) {
    deactivate();
    return;
  }
}

void Controller::updateRequired() {
  logger.warning() << "Update required";

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
  logger.debug() << "Logout";

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

  if (nextStep == BackendFailure) {
    emit readyToBackendFailure();
    return true;
  }

  return false;
}

void Controller::setState(State state) {
  logger.debug() << "Setting state:" << state;

  if (m_state != state) {
    m_state = state;
    emit stateChanged();
  }
}

qint64 Controller::time() const {
  return m_connectedTimeInUTC.secsTo(QDateTime::currentDateTimeUtc());
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
    std::function<void(const QString& serverIpv4Gateway,
                       const QString& deviceIpv4Address, uint64_t txByte,
                       uint64_t rxBytes)>&& a_callback) {
  logger.debug() << "check status";

  std::function<void(const QString& serverIpv4Gateway,
                     const QString& deviceIpv4Address, uint64_t txBytes,
                     uint64_t rxBytes)>
      callback = std::move(a_callback);

  if (m_state != StateOn && m_state != StateConfirming) {
    callback(QString(), QString(), 0, 0);
    return;
  }

  bool requestStatus = m_getStatusCallbacks.isEmpty();

  m_getStatusCallbacks.append(std::move(callback));

  if (m_impl && requestStatus) {
    m_impl->checkStatus();
  }
}

void Controller::statusUpdated(const QString& serverIpv4Gateway,
                               const QString& deviceIpv4Address,
                               uint64_t txBytes, uint64_t rxBytes) {
  logger.debug() << "Status updated";
  QList<std::function<void(const QString& serverIpv4Gateway,
                           const QString& deviceIpv4Address, uint64_t txBytes,
                           uint64_t rxBytes)>>
      list;

  list.swap(m_getStatusCallbacks);
  for (const std::function<void(
           const QString&serverIpv4Gateway, const QString&deviceIpv4Address,
           uint64_t txBytes, uint64_t rxBytes)>&func : list) {
    func(serverIpv4Gateway, deviceIpv4Address, txBytes, rxBytes);
  }
}

QList<IPAddressRange> Controller::getAllowedIPAddressRanges(
    const QList<Server>& serverList) {
  logger.debug() << "Computing the allowed ip addresses";

  bool ipv6Enabled = SettingsHolder::instance()->ipv6Enabled();

  QList<IPAddress> excludeIPv4s;
  QList<IPAddress> excludeIPv6s;
  // For multi-hop connections, the last entry in the server list is the
  // ingress node to the network of wireguard servers, and must not be
  // routed through the VPN.
  const Server& server = serverList.last();

  // filtering out the captive portal endpoint
  if (FeatureCaptivePortal::instance()->isSupported() &&
      SettingsHolder::instance()->captivePortalAlert()) {
    CaptivePortal* captivePortal = MozillaVPN::instance()->captivePortal();

    const QStringList& captivePortalIpv4Addresses =
        captivePortal->ipv4Addresses();

    for (const QString& address : captivePortalIpv4Addresses) {
      logger.debug() << "Filtering out the captive portal address" << address;
      excludeIPv4s.append(IPAddress::create(address));
    }
  }

  // filtering out the RFC1918 local area network
  if (FeatureLocalAreaAccess::instance()->isSupported() &&
      SettingsHolder::instance()->localNetworkAccess()) {
    logger.debug() << "Filtering out the local area networks (rfc 1918)";
    excludeIPv4s.append(RFC1918::ipv4());

    if (ipv6Enabled) {
      logger.debug() << "Filtering out the local area networks (rfc 4193)";
      excludeIPv6s.append(RFC4193::ipv6());
    }
  }
  if (shouldExcludeDns()) {
    // Filter out the Custom DNS Server, if the User has one.
    logger.debug() << "Filtering out the DNS address"
                   << SettingsHolder::instance()->userDNS();
    excludeIPv4s.append(
        IPAddress::create(SettingsHolder::instance()->userDNS()));
  }

  QList<IPAddressRange> list;

  if (excludeIPv4s.isEmpty()) {
    logger.debug() << "Catch all IPv4";
    list.append(IPAddressRange("0.0.0.0", 0, IPAddressRange::IPv4));
  } else {
    QList<IPAddress> allowedIPv4s{IPAddress::create("0.0.0.0/0")};

    logger.debug() << "Exclude the ingress server:" << server.ipv4AddrIn();
    excludeIPv4s.append(IPAddress::create(server.ipv4AddrIn()));

    allowedIPv4s = IPAddress::excludeAddresses(allowedIPv4s, excludeIPv4s);
    list.append(IPAddressRange::fromIPAddressList(allowedIPv4s));

    logger.debug() << "Allow the ingress server:" << server.ipv4Gateway();
    list.append(IPAddressRange(server.ipv4Gateway(), 32, IPAddressRange::IPv4));
  }

  if (ipv6Enabled) {
    if (excludeIPv6s.isEmpty()) {
      logger.debug() << "Catch all IPv6";
      list.append(IPAddressRange("::0", 0, IPAddressRange::IPv6));
    } else {
      QList<IPAddress> allowedIPv6s{IPAddress::create("::/0")};

      logger.debug() << "Exclude the ingress server:" << server.ipv6AddrIn();
      excludeIPv6s.append(IPAddress::create(server.ipv6AddrIn()));

      allowedIPv6s = IPAddress::excludeAddresses(allowedIPv6s, excludeIPv6s);
      list.append(IPAddressRange::fromIPAddressList(allowedIPv6s));

      logger.debug() << "Allow the ingress server:" << server.ipv6Gateway();
      list.append(
          IPAddressRange(server.ipv6Gateway(), 128, IPAddressRange::IPv6));
    }
  }

  return list;
}

bool Controller::shouldExcludeDns() {
  auto settings = SettingsHolder::instance();
  if (!FeatureCustomDNS::instance()->isSupported()) {
    return false;
  }
  if (settings->useGatewayDNS()) {
    return false;
  }
  auto dns = settings->userDNS();
  if (!settings->validateUserDNS(dns)) {
    return false;
  }
  // No need to filter out loopback ip addresses
  if (RFC5735::ipv4LoopbackAddressBlock().contains(QHostAddress(dns))) {
    return false;
  }
  bool isLocalDNS = RFC1918::contains(QHostAddress(dns));
  // In case we cant use lan access, no need to exclude anyway.
  if (!FeatureLocalAreaAccess::instance()->isSupported()) {
    return false;
  }

  // TODO: Uncomment this once mullvad is ready to route custom dns
  // currently we want all custom dns to not use the vpn because of this.
  // if(!isLocalDNS){
  //  return false;
  //}
  if (isLocalDNS && settings->localNetworkAccess()) {
    // DNS is lan, but we already excluded local-ip's, all good.
    return false;
  }
  return true;
}

void Controller::resetConnectionCheck() {
  m_reconnectionStep = NoReconnection;

  m_connectionCheck.stop();

  m_connectionRetry = 0;
  emit connectionRetryChanged();
}

void Controller::heartbeatCompleted() {
  if (m_reconnectionStep != ExpectHeartbeat) {
    return;
  }

  m_reconnectionStep = NoReconnection;

  // If we are still in the main state, we can try to reconnect.
  if (MozillaVPN::instance()->state() == MozillaVPN::StateMain) {
    activateInternal();
  }
}

void Controller::resetConnectedTime() {
  m_connectedTimeInUTC = QDateTime::currentDateTimeUtc();
}

QString Controller::currentLocalizedCityName() const {
  return ServerI18N::translateCityName(m_currentCountryCode, m_currentCity);
}

QString Controller::switchingLocalizedCityName() const {
  return ServerI18N::translateCityName(m_switchingCountryCode, m_switchingCity);
}
