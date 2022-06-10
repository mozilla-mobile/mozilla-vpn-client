/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ioscontroller.h"
#include "Mozilla_VPN-Swift.h"
#include "device.h"
#include "ipaddress.h"
#include "keys.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"
#include "mozillavpn.h"
#include "server.h"
#include "settingsholder.h"

#include <QByteArray>
#include <QFile>
#include <QHostAddress>

namespace {

Logger logger({LOG_IOS, LOG_CONTROLLER}, "IOSController");

// Our Swift singleton.
IOSControllerImpl* impl = nullptr;

}  // namespace

IOSController::IOSController() {
  MVPN_COUNT_CTOR(IOSController);

  logger.debug() << "created";

  Q_ASSERT(!impl);
}

IOSController::~IOSController() {
  MVPN_COUNT_DTOR(IOSController);

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
            // Just in case we are connecting, let's call disconnect.
            [impl disconnect];
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

void IOSController::activate(const HopConnection& hop, const Device* device, const Keys* keys,
                             Reason reason) {
  Q_UNUSED(device);
  Q_UNUSED(keys);

  // These features are not supported on macos/ios yet.
  Q_ASSERT(hop.m_hopindex == 0);
  Q_ASSERT(hop.m_vpnDisabledApps.isEmpty());

  logger.debug() << "IOSController activating" << hop.m_server.hostname();

  if (!impl) {
    logger.error() << "Controller not correctly initialized";
    emit disconnected();
    return;
  }

  m_serverPublicKey = hop.m_server.publicKey();

  NSMutableArray<VPNIPAddressRange*>* allowedIPAddressRangesNS =
      [NSMutableArray<VPNIPAddressRange*> arrayWithCapacity:hop.m_allowedIPAddressRanges.length()];
  for (const IPAddress& i : hop.m_allowedIPAddressRanges) {
    VPNIPAddressRange* range =
        [[VPNIPAddressRange alloc] initWithAddress:i.address().toString().toNSString()
                               networkPrefixLength:i.prefixLength()
                                            isIpv6:i.type() == QAbstractSocket::IPv6Protocol];
    [allowedIPAddressRangesNS addObject:[range autorelease]];
  }

  [impl connectWithDnsServer:hop.m_dnsServer.toString().toNSString()
           serverIpv6Gateway:hop.m_server.ipv6Gateway().toNSString()
             serverPublicKey:hop.m_server.publicKey().toNSString()
            serverIpv4AddrIn:hop.m_server.ipv4AddrIn().toNSString()
                  serverPort:hop.m_server.choosePort()
      allowedIPAddressRanges:allowedIPAddressRangesNS
                      reason:reason
             failureCallback:^() {
               logger.error() << "IOSSWiftController - connection failed";
               emit disconnected();
             }];
}

void IOSController::deactivate(Reason reason) {
  logger.debug() << "IOSController deactivated";

  if (reason != ReasonNone) {
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

  QString groupId(GROUP_ID);
  NSURL* groupPath = [[NSFileManager defaultManager]
      containerURLForSecurityApplicationGroupIdentifier:groupId.toNSString()];

  NSURL* path = [groupPath URLByAppendingPathComponent:@"networkextension.log"];

  QFile file(QString::fromNSString([path path]));
  if (!file.open(QIODevice::ReadOnly)) {
    callback("Network extension log file missing or unreadable.");
    return;
  }

  QByteArray content = file.readAll();
  callback(content);
}

void IOSController::cleanupBackendLogs() {
  QString groupId(GROUP_ID);
  NSURL* groupPath = [[NSFileManager defaultManager]
      containerURLForSecurityApplicationGroupIdentifier:groupId.toNSString()];

  NSURL* path = [groupPath URLByAppendingPathComponent:@"networkextension.log"];

  QFile file(QString::fromNSString([path path]));
  file.remove();
}
