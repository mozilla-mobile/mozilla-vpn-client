/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ioscontroller.h"
#include "Mozilla-Swift.h"
#include "controller.h"
#include "feature/feature.h"
#include "glean/generated/metrics.h"
#include "ipaddress.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/device.h"
#include "models/keys.h"
#include "models/location.h"
#include "models/server.h"
#include "mozillavpn.h"
#include "settingsholder.h"

#include <QByteArray>
#include <QFile>
#include <QHostAddress>

namespace {

Logger logger("IOSController");

// Our Swift singleton.
IOSControllerImpl* impl = nullptr;

}  // namespace

IOSController::IOSController() {
  MZ_COUNT_CTOR(IOSController);

  logger.debug() << "created";

  Q_ASSERT(!impl);
}

IOSController::~IOSController() {
  MZ_COUNT_DTOR(IOSController);

  logger.debug() << "deallocated";

  if (impl) {
    [impl dealloc];
    impl = nullptr;
  }
}

void IOSController::initialize(const Device* device, const Keys* keys) {
  Q_ASSERT(!impl);
  Q_UNUSED(device);

  logger.debug() << "Initializing Swift Controller";

  static bool creating = false;
  // No nested creation!
  Q_ASSERT(creating == false);
  creating = true;

  QByteArray key = QByteArray::fromBase64(keys->privateKey().toLocal8Bit());

  impl = [[IOSControllerImpl alloc] initWithBundleID:@VPN_NE_BUNDLEID
      privateKey:key.toNSData()
      deviceIpv4Address:device->ipv4Address().toNSString()
      deviceIpv6Address:device->ipv6Address().toNSString()
      closure:^(ConnectionState state, NSDate* date) {
        logger.debug() << "Creation completed with connection state:" << state;
        creating = false;

        switch (state) {
          case ConnectionStateError: {
            [impl dealloc];
            impl = nullptr;
            emit initialized(false, false, QDateTime());
            return;
          }
          case ConnectionStateConnected: {
            Q_ASSERT(date);
            QDateTime qtDate(QDateTime::fromNSDate(date));
            emit initialized(true, true, qtDate);
            return;
          }
          case ConnectionStateDisconnected:
            Controller* controller = MozillaVPN::instance()->controller();
            Q_ASSERT(controller);
            if (controller->state() != Controller::StateInitializing) {
              // Just in case we are connecting, let's call disconnect.
              [impl disconnect];
            }

            emit initialized(true, false, QDateTime());
            return;
        }
      }
      callback:^(BOOL a_connected) {
        logger.debug() << "State changed: " << a_connected;
        if (a_connected) {
          emit connected(m_serverPublicKey);
          return;
        }

        emit disconnected();
      }];
}

void IOSController::activate(const InterfaceConfig& config, Controller::Reason reason) {
  // These features are not supported on ios yet.
  Q_ASSERT(config.m_hopType == InterfaceConfig::SingleHop);
  Q_ASSERT(config.m_vpnDisabledApps.isEmpty());

  logger.debug() << "IOSController activating" << config.m_serverPublicKey;

  if (!impl) {
    logger.error() << "Controller not correctly initialized";

#if TARGET_OS_SIMULATOR
    if (MozillaVPN::instance()->state() == App::StateOnboarding) {
      logger.debug() << "Cannot activate VPN on a simulator. Completing onboarding.";
      MozillaVPN::instance()->onboardingCompleted();
    }
#endif

    emit disconnected();
    return;
  }

  m_serverPublicKey = config.m_serverPublicKey;

  NSMutableArray<VPNIPAddressRange*>* allowedIPAddressRangesNS = [NSMutableArray<VPNIPAddressRange*>
      arrayWithCapacity:config.m_allowedIPAddressRanges.length()];
  for (const IPAddress& i : config.m_allowedIPAddressRanges) {
    VPNIPAddressRange* range =
        [[VPNIPAddressRange alloc] initWithAddress:i.address().toString().toNSString()
                               networkPrefixLength:i.prefixLength()
                                            isIpv6:i.type() == QAbstractSocket::IPv6Protocol];
    [allowedIPAddressRangesNS addObject:[range autorelease]];
  }

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  ServerData mainServerData = MozillaVPN::instance()->controller()->currentServer();
  NSMutableArray<VPNServerData*>* serverData = [[NSMutableArray<VPNServerData*> alloc] init];

  VPNServerData* mainServer =
      [[VPNServerData alloc] initWithDns:config.m_dnsServer.toNSString()
                             ipv6Gateway:config.m_serverIpv6Gateway.toNSString()
                               publicKey:config.m_serverPublicKey.toNSString()
                              ipv4AddrIn:config.m_serverIpv4AddrIn.toNSString()
                                    port:config.m_serverPort];
  [serverData addObject:mainServer];

  // Select the best DNS - if user is using a special privacy or user-specified DNS, use that.
  // Otherwise, use the gateway for this server.
  NSString* mainServerDns = config.m_dnsServer.toNSString();
  NSString* mainServerGateway = config.m_serverIpv4Gateway.toNSString();
  BOOL isUsingNormalDns = [mainServerDns isEqualToString:mainServerGateway];

  const QList<Server> fallbackServers = mainServerData.backupServers(config.m_serverPublicKey);
  for (const Server& fallbackServer : fallbackServers) {
    NSString* dnsServer;
    if (isUsingNormalDns) {
      dnsServer = fallbackServer.ipv4Gateway().toNSString();
    } else {
      dnsServer = mainServerDns;
    }
    VPNServerData* backupServerData =
        [[VPNServerData alloc] initWithDns:dnsServer
                               ipv6Gateway:fallbackServer.ipv6Gateway().toNSString()
                                 publicKey:fallbackServer.publicKey().toNSString()
                                ipv4AddrIn:fallbackServer.ipv4AddrIn().toNSString()
                                      port:fallbackServer.choosePort()];
    [serverData addObject:backupServerData];
  }

  [impl connectWithServerData:serverData
      permitLocalNetworkFeatures:settingsHolder->localNetworkAccess()
      allowedIPAddressRanges:allowedIPAddressRangesNS
      reason:reason
      gleanDebugTag:settingsHolder->gleanDebugTagActive()
                        ? settingsHolder->gleanDebugTag().toNSString()
                        : @""
      isSuperDooperFeatureActive:Feature::get(Feature::Feature_superDooperMetrics)->isSupported()
      installationId:config.m_installationId.toNSString()
      isMissingLocalLocation:MozillaVPN::instance()->location()->countryCode().isEmpty()
      isServerLocatedInUserCountry:MozillaVPN::instance()
                                       ->serverData()
                                       ->serverLocatedInUserCountry()
      disconnectOnErrorCallback:^() {
        logger.error() << "IOSSWiftController - disconnecting";
        emit disconnected();
        // If this disconnect is due to the end of onboarding, call appropriate function.
        if (SettingsHolder::instance()->onboardingCompleted() == false) {
          logger.error() << "Error in tunnel creation, but finishing onboarding";
          MozillaVPN::instance()->onboardingCompleted();
        }
      }
      onboardingCompletedCallback:^() {
        BOOL isOnboarding = MozillaVPN::instance()->state() == App::StateOnboarding;
        if (isOnboarding) {
          logger.debug() << "Onboarding completed";
          MozillaVPN::instance()->onboardingCompleted();
        } else {
          logger.debug() << "Not onboarding";
        }
      }
      vpnConfigPermissionResponseCallback:^(BOOL granted) {
        Controller* controller = MozillaVPN::instance()->controller();
        controller->startHandshakeTimer();
      }];
}

void IOSController::deactivate() {
  logger.debug() << "IOSController deactivated";

  if (!impl) {
    logger.error() << "Controller not correctly initialized";
    emit disconnected();
    return;
  }

  [impl disconnect];
}

void IOSController::deleteOSTunnelConfig() { [impl deleteOSTunnelConfig]; }

void IOSController::checkStatus() {
  logger.debug() << "Checking status";

  if (m_checkingStatus) {
    logger.warning() << "We are still waiting for the previous status.";
    return;
  }

  if (!impl) {
    logger.error() << "Controller not correctly initialized";
    return;
  }

  m_checkingStatus = true;

  [impl checkStatusWithCallback:^(NSString* serverIpv4Gateway, NSString* deviceIpv4Address,
                                  NSString* configString) {
    QString config = QString::fromNSString(configString);

    m_checkingStatus = false;

    if (config.isEmpty()) {
      return;
    }

    uint64_t txBytes = 0;
    uint64_t rxBytes = 0;

    QStringList lines = config.split("\n");
    for (const QString& line : lines) {
      if (line.startsWith("tx_bytes=")) {
        txBytes = line.split("=")[1].toULongLong();
      } else if (line.startsWith("rx_bytes=")) {
        rxBytes = line.split("=")[1].toULongLong();
      }

      if (txBytes && rxBytes) {
        break;
      }
    }

    logger.debug() << "ServerIpv4Gateway:" << serverIpv4Gateway
                   << "DeviceIpv4Address:" << deviceIpv4Address
                   << "RxBytes:" << rxBytes << "TxBytes:" << txBytes;
    emit statusUpdated(QString::fromNSString(serverIpv4Gateway),
                       QString::fromNSString(deviceIpv4Address), txBytes, rxBytes);
  }];
}

void IOSController::forceDaemonSilentServerSwitch() { [impl silentServerSwitch]; }

void IOSController::getBackendLogs(QIODevice* device) {
  [IOSLoggerImpl getLogsWithCallback:^(NSString* logs) {
    device->write([logs UTF8String],
                  [logs lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
    device->close();
  }];
}

void IOSController::cleanupBackendLogs() { [IOSLoggerImpl clearLogs]; }
