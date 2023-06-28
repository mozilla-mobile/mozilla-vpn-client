/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ioscontroller.h"
#include "Mozilla-Swift.h"
#include "controller.h"
#include "feature.h"
#include "ipaddress.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/device.h"
#include "models/keys.h"
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
      callback:^(BOOL a_connected, NSDate* date) {
        QDateTime qtDate(QDateTime::fromNSDate(date));
        logger.debug() << "State changed: " << a_connected;
        if (a_connected) {
          emit connected(m_serverPublicKey, qtDate);
          return;
        }

        emit disconnected();
      }];
}

void IOSController::activate(const InterfaceConfig& config,
                             Controller::Reason reason) {

  // These features are not supported on ios yet.
  Q_ASSERT(config.m_hopType == "single");
  Q_ASSERT(config.m_vpnDisabledApps.isEmpty());

  logger.debug() << "IOSController activating" << config.m_serverPublicKey;

  if (!impl) {
    logger.error() << "Controller not correctly initialized";
    emit disconnected();
    return;
  }

  m_serverPublicKey = config.m_serverPublicKey;

  NSMutableArray<VPNIPAddressRange*>* allowedIPAddressRangesNS =
      [NSMutableArray<VPNIPAddressRange*> arrayWithCapacity:config.m_allowedIPAddressRanges.length()];
  for (const IPAddress& i : config.m_allowedIPAddressRanges) {
    VPNIPAddressRange* range =
        [[VPNIPAddressRange alloc] initWithAddress:i.address().toString().toNSString()
                               networkPrefixLength:i.prefixLength()
                                            isIpv6:i.type() == QAbstractSocket::IPv6Protocol];
    [allowedIPAddressRangesNS addObject:[range autorelease]];
  }

  [impl connectWithDnsServer:config.m_dnsServer.toNSString()
           serverIpv6Gateway:config.m_serverIpv6Gateway.toNSString()
             serverPublicKey:config.m_serverPublicKey.toNSString()
            serverIpv4AddrIn:config.m_serverIpv4AddrIn.toNSString()
                  serverPort:config.m_serverPort
      allowedIPAddressRanges:allowedIPAddressRangesNS
                      reason:reason
             failureCallback:^() {
               logger.error() << "IOSSWiftController - connection failed";
               emit disconnected();
             }];
}

void IOSController::deactivate(Controller::Reason reason) {
  logger.debug() << "IOSController deactivated";

  if (reason != Controller::ReasonNone) {
    logger.debug() << "We do not need to disable the VPN for switching or connection check.";
    emit disconnected();
    return;
  }

  if (!impl) {
    logger.error() << "Controller not correctly initialized";
    emit disconnected();
    return;
  }

  [impl disconnect];
}

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

    logger.debug() << "ServerIpv4Gateway:" << QString::fromNSString(serverIpv4Gateway)
                   << "DeviceIpv4Address:" << QString::fromNSString(deviceIpv4Address)
                   << "RxBytes:" << rxBytes << "TxBytes:" << txBytes;
    emit statusUpdated(QString::fromNSString(serverIpv4Gateway),
                       QString::fromNSString(deviceIpv4Address), txBytes, rxBytes);
  }];
}

void IOSController::getBackendLogs(std::function<void(const QString&)>&& a_callback) {
  std::function<void(const QString&)> callback = std::move(a_callback);

  [IOSLoggerImpl getAppexLogsWithCallback:^(NSString* logs) {
    callback(QString::fromNSString(logs));
  }];
}

void IOSController::cleanupBackendLogs() {
    [IOSLoggerImpl clearAppexLogs];
}
