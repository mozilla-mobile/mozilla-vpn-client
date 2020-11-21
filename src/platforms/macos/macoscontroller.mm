/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macoscontroller.h"
#include "Mozilla_VPN-Swift.h"
#include "device.h"
#include "ipaddressrange.h"
#include "keys.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "server.h"
#include "settingsholder.h"

#include <QByteArray>
#include <QFile>

namespace {

Logger logger({LOG_MACOS, LOG_CONTROLLER}, "MacOSController");

// Our Swift singleton.
MacOSControllerImpl* impl = nullptr;

}  // namespace

MacOSController::MacOSController() {
  MVPN_COUNT_CTOR(MacOSController);
  Q_ASSERT(!impl);
}

MacOSController::~MacOSController() {
  MVPN_COUNT_DTOR(MacOSController);

  if (impl) {
    [impl dealloc];
    impl = nullptr;
  }
}

void MacOSController::initialize(const Device* device, const Keys* keys) {
  Q_ASSERT(!impl);

  logger.log() << "Initializing Swift Controller";

  static bool creating = false;
  // No nested creation!
  Q_ASSERT(creating == false);
  creating = true;

  QByteArray key = QByteArray::fromBase64(keys->privateKey().toLocal8Bit());

  impl = [[MacOSControllerImpl alloc] initWithPrivateKey:key.toNSData()
      deviceIpv4Address:device->ipv4Address().toNSString()
      deviceIpv6Address:device->ipv6Address().toNSString()
      closure:^(ConnectionState state, NSDate* date) {
        logger.log() << "Creation completed with connection state:" << state;
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
            emit initialized(true, false, QDateTime());
            return;
        }
      }
      callback:^(BOOL a_connected) {
        logger.log() << "State changed: " << a_connected;
        if (a_connected) {
          emit connected();
          return;
        }

        emit disconnected();
      }];
}

void MacOSController::activate(const Server& server, const Device* device, const Keys* keys,
                               const QList<IPAddressRange>& allowedIPAddressRanges,
                               bool forSwitching) {
  Q_UNUSED(device);
  Q_UNUSED(keys);

  logger.log() << "MacOSController activating" << server.hostname();

  if (!impl) {
    logger.log() << "Controller not correctly initialized";
    emit disconnected();
    return;
  }

  NSMutableArray<VPNIPAddressRange*>* allowedIPAddressRangesNS =
      [NSMutableArray<VPNIPAddressRange*> arrayWithCapacity:allowedIPAddressRanges.length()];
  for (const IPAddressRange& i : allowedIPAddressRanges) {
    VPNIPAddressRange* range =
        [[VPNIPAddressRange alloc] initWithAddress:i.ipAddress().toNSString()
                               networkPrefixLength:i.range()
                                            isIpv6:i.type() == IPAddressRange::IPv6];
    [allowedIPAddressRangesNS addObject:[range autorelease]];
  }

  [impl connectWithServerIpv4Gateway:server.ipv4Gateway().toNSString()
                   serverIpv6Gateway:server.ipv6Gateway().toNSString()
                     serverPublicKey:server.publicKey().toNSString()
                    serverIpv4AddrIn:server.ipv4AddrIn().toNSString()
                          serverPort:server.choosePort()
              allowedIPAddressRanges:allowedIPAddressRangesNS
                         ipv6Enabled:SettingsHolder::instance()->ipv6Enabled()
                        forSwitching:forSwitching
                     failureCallback:^() {
                       logger.log() << "MacOSSWiftController - connection failed";
                       emit disconnected();
                     }];
}

void MacOSController::deactivate(bool forSwitching) {
  logger.log() << "MacOSController deactivated";

  if (forSwitching) {
    logger.log() << "We do not need to disable the VPN for switching.";
    emit disconnected();
    return;
  }

  if (!impl) {
    logger.log() << "Controller not correctly initialized";
    emit disconnected();
    return;
  }

  [impl disconnect];
}

void MacOSController::checkStatus() {
  logger.log() << "Checking status";

  if (m_checkingStatus) {
    logger.log() << "We are still waiting for the previous status.";
    return;
  }

  if (!impl) {
    logger.log() << "Controller not correctly initialized";
    return;
  }

  m_checkingStatus = true;

  [impl checkStatusWithCallback:^(NSString* serverIpv4Gateway, NSString* configString) {
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

    logger.log() << "ServerIpv4Gateway:" << QString::fromNSString(serverIpv4Gateway)
                 << "RxBytes:" << rxBytes << "TxBytes:" << txBytes;
    emit statusUpdated(QString::fromNSString(serverIpv4Gateway), txBytes, rxBytes);
  }];
}

void MacOSController::getBackendLogs(std::function<void(const QString&)>&& a_callback) {
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
