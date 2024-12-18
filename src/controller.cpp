/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "controller.h"

#include <QFileInfo>
#include <QNetworkInformation>

#include "app.h"
#include "apppermission.h"
#include "captiveportal/captiveportal.h"
#include "constants.h"
#include "controllerimpl.h"
#include "dnshelper.h"
#include "feature/feature.h"
#include "frontend/navigator.h"
#include "ipaddress.h"
#include "leakdetector.h"
#include "localsocketcontroller.h"
#include "logger.h"
#include "models/devicemodel.h"
#include "models/keys.h"
#include "models/server.h"
#include "models/servercountrymodel.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "networkwatcher.h"
#include "rfc/rfc1112.h"
#include "rfc/rfc1918.h"
#include "rfc/rfc4193.h"
#include "rfc/rfc4291.h"
#include "serverlatency.h"
#include "settingsholder.h"
#include "tasks/controlleraction/taskcontrolleraction.h"
#include "tasks/function/taskfunction.h"
#include "tasks/heartbeat/taskheartbeat.h"
#include "taskscheduler.h"

#if defined(MZ_FLATPAK)
#  include "platforms/linux/networkmanagercontroller.h"
#elif defined(MZ_LINUX)
#  include "platforms/linux/linuxcontroller.h"
#elif defined(MZ_IOS)
#  include "platforms/ios/ioscontroller.h"
#elif defined(MZ_ANDROID)
#  include "platforms/android/androidcontroller.h"
#elif defined(MZ_WASM)
#  include "platforms/wasm/wasmcontroller.h"
#endif

// The Mullvad proxy services are located at internal IPv4 addresses in the
// 10.124.0.0/20 address range, which is a subset of the 10.0.0.0/8 Class-A
// private address range.
constexpr const char* MULLVAD_PROXY_RANGE = "10.124.0.0";
constexpr const int MULLVAD_PROXY_RANGE_LENGTH = 20;

namespace {
Logger logger("Controller");

// X connection retries.
constexpr const int CONNECTION_MAX_RETRY = 9;
using namespace std::chrono_literals;
constexpr const auto CONFIRMING_TIMOUT = 10s;
constexpr const auto HANDSHAKE_TIMEOUT = 15s;
constexpr const auto CONNECTION_TIME_UPDATE_FREQUENCY = 1s;

Controller::Reason stateToReason(Controller::State state) {
  if (state == Controller::StateSwitching ||
      state == Controller::StateSilentSwitching) {
    return Controller::ReasonSwitching;
  }

  if (state == Controller::StateConfirming) {
    return Controller::ReasonConfirming;
  }

  if (state == Controller::StateOnboarding) {
    return Controller::ReasonOnboarding;
  }

  return Controller::ReasonNone;
}
}  // namespace

Controller::Controller() {
  MZ_COUNT_CTOR(Controller);

  m_connectingTimer.setSingleShot(true);
  m_handshakeTimer.setSingleShot(true);

  connect(&m_timer, &QTimer::timeout, this, &Controller::timerTimeout);

  connect(&m_connectingTimer, &QTimer::timeout, this, [this]() {
    m_enableDisconnectInConfirming = true;
    emit enableDisconnectInConfirmingChanged();
  });

  connect(&m_handshakeTimer, &QTimer::timeout, this,
          &Controller::handshakeTimeout);

  LogHandler::instance()->registerLogSerializer(this);
}

Controller::~Controller() {
  LogHandler::instance()->unregisterLogSerializer(this);
  MZ_COUNT_DTOR(Controller);
}

// Check if we can use a LocalSocketController and return its path if so.
QString Controller::useLocalSocketPath() const {
#ifndef MZ_WASM
  // The control socket can be overriden for testing.
  QString path = qEnvironmentVariable("MVPN_CONTROL_SOCKET");
  if (!Constants::inProduction() && !path.isEmpty()) {
    return path;
  }
#endif

#if defined(MZ_MACOS)
  // MacOS had a path change, so check both /tmp/ and /var/.
  if (QFileInfo::exists(Constants::MACOS_DAEMON_VAR_PATH)) {
    return Constants::MACOS_DAEMON_VAR_PATH;
  } else {
    return Constants::MACOS_DAEMON_TMP_PATH;
  }
#elif defined(MZ_WINDOWS)
  return Constants::WINDOWS_DAEMON_PATH;
#elif defined(MZ_IOS)
  // The IOS simulator also uses a mocked daemon.
  bool isSimDevice = !qEnvironmentVariable("SIMULATOR_DEVICE_NAME").isEmpty();
  if (isSimDevice && !path.isEmpty()) {
    return path;
  }
#endif

  // Otherwise, we will need some other controller.
  return QString();
}

void Controller::initialize() {
  logger.debug() << "Initializing the controller";

  if (m_state != Controller::StateInitializing) {
    setState(Controller::StateInitializing);
  }

  // Let's delete the previous controller before creating a new one.
  if (m_impl) {
    m_impl.reset(nullptr);
  }

  m_serverData = *MozillaVPN::instance()->serverData();
  m_nextServerData = *MozillaVPN::instance()->serverData();

  QString path = useLocalSocketPath();
  if (!path.isEmpty()) {
    m_impl.reset(new LocalSocketController(path));
  } else {
    // We must use a specialized platform controller
#if defined(MZ_FLATPAK)
    m_impl.reset(new NetworkManagerController());
#elif defined(MZ_LINUX)
    m_impl.reset(new LinuxController());
#elif defined(MZ_IOS)
    m_impl.reset(new IOSController());
#elif defined(MZ_ANDROID)
    m_impl.reset(new AndroidController());
#elif defined(MZ_WASM)
    m_impl.reset(new WasmController());
#else
    qCritical() << "No platform controller available for "
                << Constants::PLATFORM_NAME;
#endif
  }

  connect(m_impl.get(), &ControllerImpl::connected, this,
          &Controller::connected);
  connect(m_impl.get(), &ControllerImpl::disconnected, this,
          &Controller::disconnected);
  connect(m_impl.get(), &ControllerImpl::initialized, this,
          &Controller::implInitialized);
  connect(m_impl.get(), &ControllerImpl::statusUpdated, this,
          &Controller::statusUpdated);
  connect(this, &Controller::stateChanged, this,
          &Controller::maybeEnableDisconnectInConfirming);

  connect(&m_pingCanary, &PingHelper::pingSentAndReceived, this, [this]() {
    m_pingCanary.stop();
    m_pingReceived = true;
    logger.info() << "Canary Ping Succeeded";
  });

  MozillaVPN* vpn = MozillaVPN::instance();

  const Device* device = vpn->deviceModel()->currentDevice(vpn->keys());
  m_impl->initialize(device, vpn->keys());

  connect(SettingsHolder::instance(), &SettingsHolder::serverDataChanged, this,
          &Controller::serverDataChanged);

  connect(LogHandler::instance(), &LogHandler::cleanupLogsNeeded, this,
          &Controller::cleanupBackendLogs);
}

void Controller::implInitialized(bool status, bool a_connected,
                                 const QDateTime& connectionDate) {
  logger.debug() << "Connection Manager initialized with status:" << status
                 << "connected:" << a_connected
                 << "connectionDate:" << connectionDate.toString();

  if (!status) {
    REPORTERROR(ErrorHandler::ControllerError, "controller");
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
    m_connectedTimeInUTC = connectionDate.isValid()
                               ? connectionDate.toUTC()
                               : QDateTime::currentDateTimeUtc();
    emit timeChanged();
    m_timer.start(CONNECTION_TIME_UPDATE_FREQUENCY);
  }
}

qint64 Controller::time() const {
  if (m_connectedTimeInUTC.isValid()) {
    return m_connectedTimeInUTC.secsTo(QDateTime::currentDateTimeUtc());
  }
  return 0;
}

void Controller::timerTimeout() {
  Q_ASSERT(m_state == StateOn || m_state == StateOnPartial);
#ifdef MZ_IOS
  // When locking an iOS device with an app in the foreground, the app's JS
  // runtime is stopped pretty quick by the system. For this VPN app, that
  // caused a crash here when Qt tried to pass this emit off to QML, as QML
  // is JS and is no longer available. This check is to prevent that crash
  // when the device is locked.
  if (QGuiApplication::applicationState() ==
      Qt::ApplicationState::ApplicationActive) {
    emit timeChanged();
  }
#else
  emit timeChanged();
#endif
}

void Controller::quit() {
  logger.debug() << "Quitting";

  if (m_state == StateInitializing || m_state == StateOff) {
    m_nextStep = Quit;
    emit readyToQuit();
    return;
  }

  m_nextStep = Quit;

  if (m_state == StateOn || m_state == StateOnPartial ||
      m_state == StateSwitching || m_state == StateSilentSwitching ||
      m_state == StateConnecting) {
    deactivate();
    return;
  }
}

void Controller::forceDaemonCrash() {
  if (m_impl) {
    m_impl->forceDaemonCrash();
  }
}

void Controller::deleteOSTunnelConfig() {
  if (m_impl) {
    m_impl->deleteOSTunnelConfig();
  }
}

void Controller::startHandshakeTimer() {
  m_handshakeTimer.start(HANDSHAKE_TIMEOUT);
}

void Controller::handshakeTimeout() {
  logger.debug() << "Timeout while waiting for handshake";

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(!m_activationQueue.isEmpty());

  // Block the offending server and try again.
  InterfaceConfig& hop = m_activationQueue.first();
  vpn->serverLatency()->setCooldown(
      hop.m_serverPublicKey, Constants::SERVER_UNRESPONSIVE_COOLDOWN_SEC);

  if (m_nextStep == Quit || m_nextStep == Disconnect || m_nextStep == Update) {
    deactivate();
    return;
  }

  // Try again, again if there are sufficient retries left.
  ++m_connectionRetry;
  emit connectionRetryChanged();
  logger.info() << "Connection attempt " << m_connectionRetry;
  InterfaceConfig& exit_hop = m_activationQueue.last();
  Q_ASSERT(exit_hop.m_hopType != InterfaceConfig::HopType::MultiHopEntry);

  if (m_connectionRetry == 1) {
    logger.info() << "Connection Attempt: Using Port 53 Option this time.";
    // On the first retry, opportunisticly try again using the port 53
    // option enabled, if that feature is disabled.
    activateInternal(ForceDNSPort, RandomizeServerSelection, m_initiator);
    return;
  } else if (m_connectionRetry < CONNECTION_MAX_RETRY) {
    activateInternal(DoNotForceDNSPort, RandomizeServerSelection, m_initiator);
    return;
  }

  // Otherwise, the give up and report the location as unavailable.
  logger.error() << "Connection retries exhausted, giving up";
  serverUnavailable();
}

qint64 Controller::connectionTimestamp() const {
  switch (m_state) {
    case Controller::State::StateConfirming:
      [[fallthrough]];
    case Controller::State::StateConnecting:
      [[fallthrough]];
    case Controller::State::StateDisconnecting:
      [[fallthrough]];
    case Controller::State::StateInitializing:
      [[fallthrough]];
    case Controller::State::StateOff:
      [[fallthrough]];
    case Controller::State::StateOnboarding:
      return 0;
    case Controller::State::StateOn:
      [[fallthrough]];
    case Controller::State::StateOnPartial:
      [[fallthrough]];
    case Controller::State::StateSilentSwitching:
      [[fallthrough]];
    case Controller::State::StateSwitching:
      if (m_connectedTimeInUTC.isValid()) {
        return m_connectedTimeInUTC.toMSecsSinceEpoch();
      }
      const QDateTime& initialTimeStampForExtension =
          QDateTime::currentDateTimeUtc();
      return initialTimeStampForExtension.toMSecsSinceEpoch();
  }
  Q_UNREACHABLE();
}

void Controller::serverUnavailable() {
  logger.info() << "Server Unavailable - Ping succeeded: " << m_pingReceived;

  emit readyToServerUnavailable(m_pingReceived);
  return;
}

void Controller::updateRequired() {
  logger.warning() << "Update required";

  if (m_state == StateOff) {
    emit readyToUpdate();
    return;
  }

  m_nextStep = Update;

  if (m_state == StateOn || m_state == StateOnPartial) {
    deactivate();
    return;
  }
}

void Controller::activateInternal(
    DNSPortPolicy dnsPort,
    ServerSelectionPolicy serverSelectionPolicy = RandomizeServerSelection,
    ActivationPrincipal initiator = ClientUser) {
  logger.debug() << "Activation internal";
  Q_ASSERT(m_impl);
  m_initiator = initiator;

  clearConnectedTime();
  m_handshakeTimer.stop();
  m_activationQueue.clear();

  Server exitServer =
      serverSelectionPolicy == DoNotRandomizeServerSelection &&
              !m_serverData.exitServerPublicKey().isEmpty()
          ? MozillaVPN::instance()->serverCountryModel()->server(
                m_serverData.exitServerPublicKey())
          : Server::weightChooser(m_serverData.exitServers());
  if (!exitServer.initialized()) {
    logger.error() << "Empty exit server list in state" << m_state;
    serverUnavailable();
    return;
  }

  MozillaVPN* vpn = MozillaVPN::instance();
  const Device* device = vpn->deviceModel()->currentDevice(vpn->keys());
  if (!device) {
    logger.warning() << "No current device. Aborting activation.";
    m_nextStep = Disconnect;
    return;
  }
  SettingsHolder* settingsHolder = SettingsHolder::instance();

  auto allowedIPList = initiator == ExtensionUser
                           ? getExtensionProxyAddressRanges(exitServer)
                           : getAllowedIPAddressRanges(exitServer);
  // Prepare the exit server's connection data.
  InterfaceConfig exitConfig;
  exitConfig.m_privateKey = vpn->keys()->privateKey();
  exitConfig.m_deviceIpv4Address = device->ipv4Address();
  exitConfig.m_deviceIpv6Address = device->ipv6Address();
  exitConfig.m_serverIpv4Gateway = exitServer.ipv4Gateway();
  exitConfig.m_serverIpv6Gateway = exitServer.ipv6Gateway();
  exitConfig.m_serverPublicKey = exitServer.publicKey();
  exitConfig.m_serverIpv4AddrIn = exitServer.ipv4AddrIn();
  exitConfig.m_serverIpv6AddrIn = exitServer.ipv6AddrIn();
  exitConfig.m_serverPort = exitServer.choosePort();
  exitConfig.m_allowedIPAddressRanges = allowedIPList;
  exitConfig.m_dnsServer = DNSHelper::getDNS(exitServer.ipv4Gateway());
#if defined(MZ_ANDROID) || defined(MZ_IOS)
  exitConfig.m_installationId = settingsHolder->installationId();
#endif
  logger.debug() << "DNS Set" << exitConfig.m_dnsServer;

  // Splittunnel-feature could have been disabled due to a driver conflict.
  if (Feature::get(Feature::Feature_splitTunnel)->isSupported()) {
    exitConfig.m_vpnDisabledApps = settingsHolder->vpnDisabledApps();
  }
  if (Feature::get(Feature::Feature_alwaysPort53)->isSupported()) {
    dnsPort = ForceDNSPort;
  }

  // For single-hop connections, exclude the entry server
  if (!Feature::get(Feature::Feature_multiHop)->isSupported() ||
      !m_serverData.multihop()) {
    logger.info() << "Activating single hop";
    exitConfig.m_hopType = InterfaceConfig::SingleHop;

    // If requested, force the use of port 53/DNS.
    if (dnsPort == ForceDNSPort) {
      logger.info() << "Forcing port 53";
      exitConfig.m_serverPort = 53;
    }
  }
  // For controllers that support multiple hops, create a queue of connections.
  // The entry server should start first, followed by the exit server.
  else if (m_impl->multihopSupported()) {
    logger.info() << "Activating multi-hop (through platform controller)";
    exitConfig.m_hopType = InterfaceConfig::MultiHopExit;

    Server entryServer =
        serverSelectionPolicy == DoNotRandomizeServerSelection &&
                !m_serverData.entryServerPublicKey().isEmpty()
            ? MozillaVPN::instance()->serverCountryModel()->server(
                  m_serverData.entryServerPublicKey())
            : Server::weightChooser(m_serverData.entryServers());

    if (!entryServer.initialized()) {
      logger.error() << "Empty entry server list in state" << m_state;
      serverUnavailable();
      return;
    }

    InterfaceConfig entryConfig;
    entryConfig.m_privateKey = vpn->keys()->privateKey();
    entryConfig.m_deviceIpv4Address = device->ipv4Address();
    entryConfig.m_deviceIpv6Address = device->ipv6Address();
    entryConfig.m_serverPublicKey = entryServer.publicKey();
    entryConfig.m_serverIpv4AddrIn = entryServer.ipv4AddrIn();
    entryConfig.m_serverIpv6AddrIn = entryServer.ipv6AddrIn();
    entryConfig.m_serverPort = entryServer.choosePort();
    entryConfig.m_hopType = InterfaceConfig::MultiHopEntry;
    entryConfig.m_allowedIPAddressRanges.append(
        IPAddress(exitServer.ipv4AddrIn()));
    entryConfig.m_allowedIPAddressRanges.append(
        IPAddress(exitServer.ipv6AddrIn()));

    // If requested, force the use of port 53/DNS.
    if (dnsPort == ForceDNSPort) {
      logger.info() << "Forcing port 53";
      entryConfig.m_serverPort = 53;
    }

    m_activationQueue.append(entryConfig);
  }
  // Otherwise, we can approximate multihop support by redirecting the
  // connection to the exit server via the multihop port.
  else {
    logger.info() << "Activating multi-hop (through multihop port)";
    exitConfig.m_hopType = InterfaceConfig::SingleHop;

    Server entryServer =
        serverSelectionPolicy == DoNotRandomizeServerSelection &&
                !m_serverData.entryServerPublicKey().isEmpty()
            ? MozillaVPN::instance()->serverCountryModel()->server(
                  m_serverData.entryServerPublicKey())
            : Server::weightChooser(m_serverData.entryServers());

    if (!entryServer.initialized()) {
      logger.error() << "Empty entry server list in state" << m_state;
      serverUnavailable();
      return;
    }

    // NOTE: For platforms without multihop support, we cannot emulate multihop
    // and use port 53 at the same time. If the user has selected both options
    // then let's choose multihop.
    exitConfig.m_serverPort = exitServer.multihopPort();
    exitConfig.m_serverIpv4AddrIn = entryServer.ipv4AddrIn();
    exitConfig.m_serverIpv6AddrIn = entryServer.ipv6AddrIn();
  }

  m_activationQueue.append(exitConfig);
  m_serverData.setEntryServerPublicKey(
      m_activationQueue.first().m_serverPublicKey);
  m_serverData.setExitServerPublicKey(
      m_activationQueue.last().m_serverPublicKey);

  m_pingReceived = false;
  m_pingCanary.start(m_activationQueue.first().m_serverIpv4AddrIn, "0.0.0.0/0");
  logger.info() << "Canary Ping Started";
  activateNext();
}

void Controller::clearConnectedTime() {
  if (!isSwitchingServer) {
    m_connectedTimeInUTC = QDateTime();
    emit timeChanged();
  }
  m_timer.stop();
}

// static
Controller::IPAddressList Controller::getExcludedIPAddressRanges() {
  QList<IPAddress> excludeIPv4s;
  QList<IPAddress> excludeIPv6s;

  // filtering out the RFC1918 local area network
  logger.debug() << "Filtering out the local area networks (rfc 1918)";
  excludeIPv4s.append(RFC1918::ipv4());

  logger.debug() << "Filtering out the local area networks";
  excludeIPv6s.append(RFC4193::ipv6());
  excludeIPv6s.append(RFC4291::ipv6LinkLocalAddressBlock());

  logger.debug() << "Filtering out multicast addresses";
  excludeIPv4s.append(RFC1112::ipv4MulticastAddressBlock());
  excludeIPv6s.append(RFC4291::ipv6MulticastAddressBlock());

  return IPAddressList{
      .v6 = excludeIPv6s,
      .v4 = excludeIPv4s,
  };
}

// static
QList<IPAddress> Controller::getAllowedIPAddressRanges(
    const Server& exitServer) {
  logger.debug() << "Computing the allowed IP addresses";

  QList<IPAddress> list;

  logger.debug() << "Catch all IPv4";
  list.append(IPAddress("0.0.0.0/0"));

  logger.debug() << "Catch all IPv6";
  list.append(IPAddress("::0/0"));

  // Allow access to the internal gateway addresses.
  logger.debug() << "Allow the IPv4 gateway:" << exitServer.ipv4Gateway();
  list.append(IPAddress(QHostAddress(exitServer.ipv4Gateway()), 32));
  logger.debug() << "Allow the IPv6 gateway:" << exitServer.ipv6Gateway();
  list.append(IPAddress(QHostAddress(exitServer.ipv6Gateway()), 128));

  // Ensure that the Mullvad proxy services are always allowed.
  list.append(
      IPAddress(QHostAddress(MULLVAD_PROXY_RANGE), MULLVAD_PROXY_RANGE_LENGTH));

  return list;
}

// static
QList<IPAddress> Controller::getExtensionProxyAddressRanges(
    const Server& exitServer) {
  auto const dns = DNSHelper::getDNSDetails(exitServer.ipv4Gateway());
  if (dns.dnsType == "Default" || dns.dnsType == "Custom") {
    return {IPAddress(QHostAddress(exitServer.ipv4Gateway()), 32),
            IPAddress(QHostAddress(exitServer.ipv6Gateway()), 128),
            IPAddress(QHostAddress{MULLVAD_PROXY_RANGE},
                      MULLVAD_PROXY_RANGE_LENGTH)};
  }
  return {
      IPAddress(QHostAddress(exitServer.ipv4Gateway()), 32),
      IPAddress(QHostAddress(exitServer.ipv6Gateway()), 128),
      IPAddress(QHostAddress{MULLVAD_PROXY_RANGE}, MULLVAD_PROXY_RANGE_LENGTH),
      IPAddress(QHostAddress(dns.ipAddress), 32),
  };
}

void Controller::activateNext() {
  MozillaVPN* vpn = MozillaVPN::instance();
  const Device* device = vpn->deviceModel()->currentDevice(vpn->keys());
  if (device == nullptr) {
    REPORTERROR(ErrorHandler::AuthenticationError, "controller");
    vpn->reset(false);
    return;
  }
  const InterfaceConfig& config = m_activationQueue.first();

  logger.debug() << "Activating peer" << logger.keys(config.m_serverPublicKey);

// Mobile platforms will begin handshake timer once we know the VPN
// configuration has been set with the system (to avoid persistently
// re-triggering the system prompt - VPN-5532)
#if defined(MZ_LINUX) || defined(MZ_WINDOWS) || defined(MZ_MACOS)
  startHandshakeTimer();
#endif

  m_impl->activate(config, stateToReason(m_state));

  if (m_initiator == ExtensionUser) {
    return;
  }

  if ((m_state != StateSilentSwitching) && (m_state != StateOnboarding)) {
    // Move to the StateConfirming if we are awaiting any connection handshakes
    setState(StateConfirming);
  }
}

void Controller::setState(State state) {
  if (m_state == state) {
    return;
  }
  logger.debug() << "Setting state:" << state;
  m_state = state;
  emit stateChanged();
}

Controller::State Controller::state() const { return m_state; }

bool Controller::silentSwitchServers(
    ServerCoolDownPolicyForSilentSwitch serverCoolDownPolicy) {
  logger.debug() << "Silently switch servers" << serverCoolDownPolicy;

  if (m_state != StateOn) {
    logger.warning() << "Cannot silent switch if not on";
    return false;
  }

  if (serverCoolDownPolicy == eServerCoolDownNeeded) {
    // Set a cooldown timer on the current server.
    QList<Server> servers = m_serverData.exitServers();
    Q_ASSERT(!servers.isEmpty());

    if (servers.length() <= 1) {
      logger.warning()
          << "Cannot silent switch servers because there is only one available";
      return false;
    }

    MozillaVPN::instance()->serverLatency()->setCooldown(
        m_serverData.exitServerPublicKey(),
        Constants::SERVER_UNRESPONSIVE_COOLDOWN_SEC);
  }

  isSwitchingServer = true;
  m_nextServerData = m_serverData;
  m_nextServerSelectionPolicy = serverCoolDownPolicy == eServerCoolDownNeeded
                                    ? RandomizeServerSelection
                                    : DoNotRandomizeServerSelection;

  clearConnectedTime();
  clearRetryCounter();

  logger.debug() << "Switching to a different server";

  setState(StateSilentSwitching);
  deactivate();

  return true;
}

void Controller::clearRetryCounter() {
  m_connectionRetry = 0;
  emit connectionRetryChanged();
}

void Controller::connected(const QString& pubkey) {
  logger.debug() << "handshake completed with:" << logger.keys(pubkey);
  if (m_activationQueue.isEmpty()) {
    if (m_serverData.exitServerPublicKey() != pubkey) {
      logger.warning() << "Unexpected handshake: no pending connections.";
      return;
    }
    // Continue anyways if the VPN service was activated externally.
    logger.info() << "Unexpected handshake: external VPN activation.";
  } else if (m_activationQueue.first().m_serverPublicKey != pubkey) {
    logger.warning() << "Unexpected handshake: public key mismatch.";
    return;
  } else {
    // Start the next connection if there is more work to do.
    m_activationQueue.removeFirst();
    if (!m_activationQueue.isEmpty()) {
      activateNext();
      return;
    }
  }
  m_handshakeTimer.stop();
  m_pingCanary.stop();

  // Clear the retry counter after all connections have succeeded.
  m_connectionRetry = 0;
  emit connectionRetryChanged();

  if (m_state == StateOn || m_state == StateOnPartial) {
    // The only place StateOn is set is in this function, Controller::connected.
    // If this function is called when the state is already StateOn, it is
    // because there was a silent server switch initiated from the daemon. (Only
    // mobile clients have this feature.) This is a rare occurance - if the app
    // is open, the app's silent server switch is likely to activate before the
    // daemon's silent server switch would activate. However, there is a slight
    // chance that the app is open AND the daemon initiates a silently server
    // switch. In this case, we need to belatedly set isSwitchingServer to
    // prevent resetting the timer and recording telemetry for the start of a
    // new session.
    isSwitchingServer = true;
  }

  // We have succesfully completed all pending connections.
  logger.debug() << "Connected from state:" << m_state;
  if (m_initiator == ExtensionUser) {
    setState(StateOnPartial);
  } else {
    setState(StateOn);
  }
  resetConnectedTime();

  if (isSwitchingServer == false) {
    logger.debug() << "Collecting telemetry for new session.";
    emit recordConnectionStartTelemetry();
  } else {
    logger.debug() << "Connection happened due to server switch. Not "
                      "collecting telemetry.";
  }

  if (m_nextStep == Quit || m_nextStep == Disconnect || m_nextStep == Update) {
    deactivate();
    return;
  }
}

void Controller::resetConnectedTime() {
  if (isSwitchingServer == false) {
    m_connectedTimeInUTC = QDateTime::currentDateTimeUtc();
  }
  emit timeChanged();
  m_timer.start(CONNECTION_TIME_UPDATE_FREQUENCY);
}

void Controller::disconnected() {
  logger.debug() << "Disconnected from state:" << m_state;

  m_pingCanary.stop();
  m_handshakeTimer.stop();
  m_activationQueue.clear();
  clearConnectedTime();
  clearRetryCounter();

  NextStep nextStep = m_nextStep;

  // Mobile onboarding is completed when we receive the disconnected signal.
  if (m_state == StateOnboarding) {
    logger.debug() << "Onboarding completed";
    MozillaVPN::instance()->onboardingCompleted();
  }

  if (processNextStep()) {
    setState(StateOff);
    return;
  }

  if (nextStep == None &&
      (m_state == StateSilentSwitching || m_state == StateSwitching)) {
    // If we are only silently switching, keep the iniator
    // Else move the iniator to Client User
    // as the extension cannot switch servers.
    auto target_iniator = m_state == StateSilentSwitching
                              ? m_initiator
                              : ActivationPrincipal::ClientUser;
    activate(m_nextServerData, target_iniator, m_nextServerSelectionPolicy);
    return;
  }

  // Need this StateConfirming check to prevent recording telemetry during
  // Android onboarding.
  if (m_state != StateConfirming) {
    emit recordConnectionEndTelemetry();
  }
  m_initiator = Null;
  setState(StateOff);
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
  return false;
}

void Controller::maybeEnableDisconnectInConfirming() {
  if (m_state == StateConfirming) {
    m_enableDisconnectInConfirming = false;
    emit enableDisconnectInConfirmingChanged();
    m_connectingTimer.start(CONFIRMING_TIMOUT);
  } else {
    m_enableDisconnectInConfirming = false;
    emit enableDisconnectInConfirmingChanged();
    m_connectingTimer.stop();
  }
}

bool Controller::silentServerSwitchingSupported() const {
  return m_impl->silentServerSwitchingSupported();
}

void Controller::serializeLogs(
    std::function<void(const QString& name, const QString& logs)>&&
        a_callback) {
  std::function<void(const QString& name, const QString&)> callback =
      std::move(a_callback);

  if (!m_impl) {
    callback("Mozilla VPN backend logs", QString());
    return;
  }

  m_impl->getBackendLogs([callback](const QString& logs) {
    callback("Mozilla VPN backend logs", logs);
  });
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
                           uint64_t rxBytes)> >
      list;

  list.swap(m_getStatusCallbacks);
  for (const std::function<void(
           const QString& serverIpv4Gateway, const QString& deviceIpv4Address,
           uint64_t txBytes, uint64_t rxBytes)>&func : list) {
    func(serverIpv4Gateway, deviceIpv4Address, txBytes, rxBytes);
  }
}

void Controller::captivePortalGone() {
  m_portalDetected = false;
  logger.info() << "Captive-Portal Gone, next activation will not show prompt";
}

void Controller::captivePortalPresent() {
  if (m_portalDetected) {
    return;
  }
  m_portalDetected = true;
  logger.info() << "Captive-Portal Present, next activation will show prompt";
}

void Controller::serverDataChanged() {
  if (m_state == StateOff) {
    logger.debug() << "Server data changed but we are off";
    return;
  }

  TaskScheduler::deleteTasks();
  TaskScheduler::scheduleTask(
      new TaskControllerAction(TaskControllerAction::eSwitch));
}

bool Controller::switchServers(const ServerData& serverData) {
  if (m_state == StateOff) {
    logger.debug() << "Server data changed but we are off";
    return false;
  }

  isSwitchingServer = true;
  m_nextServerData = serverData;
  m_nextServerSelectionPolicy = RandomizeServerSelection;

  clearConnectedTime();
  clearRetryCounter();

  logger.debug() << "Switching to a different server";

  setState(StateSwitching);
  deactivate();

  return true;
}

QString Controller::currentServerString() const {
  return QString("%1-%2-%3-%4")
      .arg(m_serverData.exitCountryCode(), m_serverData.exitCityName(),
           m_serverData.entryCountryCode(), m_serverData.entryCityName());
}

// These will eventually go back to the controller but to get the code working
// for now they will reside here

bool Controller::activate(const ServerData& serverData,
                          ActivationPrincipal initiator,
                          ServerSelectionPolicy serverSelectionPolicy) {
  logger.debug() << "Activation" << m_state;
  if (m_state != Controller::StateOff &&
      m_state != Controller::StateOnPartial &&
      m_state != Controller::StateSwitching &&
      m_state != Controller::StateSilentSwitching) {
    logger.debug() << "Already connected";
    return false;
  }

  isSwitchingServer = (m_state == Controller::StateSwitching ||
                       m_state == Controller::StateSilentSwitching);
  logger.debug() << "Set isSwitchingServer to" << isSwitchingServer;

  m_serverData = serverData;
  emit currentServerChanged();

  if (m_state == Controller::StateOff) {
    if (m_portalDetected) {
      emit activationBlockedForCaptivePortal();
      Navigator::instance()->requestScreen(MozillaVPN::ScreenCaptivePortal);

      m_portalDetected = false;
      return true;
    }

    // If we are in the onboarding state, this connection is being made just
    // to establish system permissions. We don't actually want to connect to
    // a server.
    if (App::instance()->state() == App::StateOnboarding) {
      setState(StateOnboarding);
      clearRetryCounter();
      activateInternal(DoNotForceDNSPort, RandomizeServerSelection, ClientUser);
      return true;
    }

    if (Feature::get(Feature::Feature_checkConnectivityOnActivation)
            ->isSupported()) {
      // Ensure that the device is connected to the Internet.
      if (MozillaVPN::instance()->networkWatcher()->getReachability() ==
          QNetworkInformation::Reachability::Disconnected) {
        logger.debug() << "Internet probe failed during controller activation. "
                          "Device has no network connectivity.";
        m_isDeviceConnected = false;
        emit isDeviceConnectedChanged();
        return false;
      }
      if (!m_isDeviceConnected) {
        m_isDeviceConnected = true;
        emit isDeviceConnectedChanged();
      }
    }

    // Set up a network request to check the subscription status.
    // "task" is an empty task function which is being used to
    // replicate the behavior of a TaskAccount.
    TaskFunction* task = new TaskFunction([]() {});
    NetworkRequest* request = new NetworkRequest(task, 200);
    request->auth();
    request->get(Constants::apiUrl(Constants::Account));

    connect(request, &NetworkRequest::requestFailed, this,
            [this, serverSelectionPolicy, initiator](
                QNetworkReply::NetworkError error, const QByteArray&) {
              logger.error() << "Account request failed" << error;
              REPORTNETWORKERROR(error, ErrorHandler::DoNotPropagateError,
                                 "PreActivationSubscriptionCheck");

              // Check if the error propagation has changed the Mozilla VPN
              // state. Continue only if the user is still authenticated and
              // subscribed.
              if (App::instance()->state() != App::StateMain) {
                return;
              }

              if (initiator != ExtensionUser) {
                setState(StateConnecting);
              }

              clearRetryCounter();
              activateInternal(DoNotForceDNSPort, serverSelectionPolicy,
                               initiator);
            });

    connect(request, &NetworkRequest::requestCompleted, this,
            [this, serverSelectionPolicy, initiator](const QByteArray& data) {
              MozillaVPN::instance()->accountChecked(data);

              if (initiator != ExtensionUser) {
                setState(StateConnecting);
              }

              clearRetryCounter();
              activateInternal(DoNotForceDNSPort, serverSelectionPolicy,
                               initiator);
            });

    connect(request, &QObject::destroyed, task, &QObject::deleteLater);
    return true;
  }

  // The next few lines are not run on iOS to prevent a bad bug.
  // These 3 lines were added in
  // https://github.com/mozilla-mobile/mozilla-vpn-client/pull/9639, and caused
  // a bug on iOS where server switches stopped working. See more details in
  // VPN-6495.
#ifndef MZ_IOS
  if (initiator != ExtensionUser) {
    setState(StateConnecting);
  }
#endif

  clearRetryCounter();
  activateInternal(DoNotForceDNSPort, serverSelectionPolicy, initiator);
  return true;
}

bool Controller::deactivate(ActivationPrincipal user) {
  logger.debug() << "Deactivation" << m_state;
  if (m_initiator > user) {
    // i.e the Firefox Extension cannot deativate the
    // vpn if we are in full device protection.
    logger.warning()
        << "ActivationPrincipal does not have permission allowed to deactivate";
    return false;
  }

  if (m_state != StateOn && m_state != StateOnPartial &&
      m_state != StateSwitching && m_state != StateSilentSwitching &&
      m_state != StateConfirming && m_state != StateConnecting) {
    logger.warning() << "Already disconnected";
    return false;
  }

  // In the event user switches from one network to a different network with a
  // captive portal, they will experience "No Signal". Upon deactivating the VPN
  // a "Captive Portal Detected" screen will be presented to inform the user of
  // the underlying issue.
  if (m_portalDetected) {
    emit activationBlockedForCaptivePortal();
    Navigator::instance()->requestScreen(MozillaVPN::ScreenCaptivePortal);

    m_portalDetected = false;
  }

  if (m_state == StateOn || m_state == StateOnPartial ||
      m_state == StateConfirming || m_state == StateConnecting) {
    setState(StateDisconnecting);
  }

  emit recordDataTransferTelemetry();

  m_pingCanary.stop();
  m_handshakeTimer.stop();
  m_activationQueue.clear();
  clearConnectedTime();
  clearRetryCounter();

  Q_ASSERT(m_impl);
  m_impl->deactivate(stateToReason(m_state));
  return true;
}

void Controller::forceDaemonSilentServerSwitch() {
#ifdef MZ_MOBILE
  if (m_impl) {
    logger.debug() << "Sending server switch message to mobile daemon";
    m_impl->forceDaemonSilentServerSwitch();
  } else {
    logger.error() << "No server switch message sent to mobile daemon "
                      "- no controller found";
  }
#else
  logger.debug() << "Server switch debug feature only available for iOS "
                    "and Android. Not sending message.";
#endif
}
