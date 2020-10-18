/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macoscontroller.h"
#include "Mozilla_VPN-Swift.h"
#include "captiveportal/captiveportal.h"
#include "device.h"
#include "keys.h"
#include "logger.h"
#include "mozillavpn.h"
#include "server.h"

#include <QByteArray>
#include <QFile>

namespace {

Logger logger(LOG_CONTROLLER, "MacOSController");

// Our Swift singleton.
MacOSControllerImpl *impl = nullptr;

} // namespace

void MacOSController::initialize(const Device *device, const Keys *keys) {
    Q_ASSERT(!impl);

    logger.log() << "Initializing Swift Controller";

    static bool creating = false;
    // No nested creation!
    Q_ASSERT(creating == false);
    creating = true;

    QByteArray key = QByteArray::fromBase64(keys->privateKey().toLocal8Bit());

    impl = [[MacOSControllerImpl alloc] initWithPrivateKey:key.toNSData()
        ipv4Address:device->ipv4Address().toNSString()
        ipv6Address:device->ipv6Address().toNSString()
        ipv6Enabled:MozillaVPN::instance()->settingsHolder()->ipv6Enabled()
        closure:^(ConnectionState state, NSDate *date) {
            logger.log() << "Creation completed with connection state:" << state;
            creating = false;

            switch (state) {
            case ConnectionStateError: {
                [impl dealloc];
                impl = nullptr;
                emit initialized(false, Controller::StateOff, QDateTime());
                return;
            }
            case ConnectionStateConnected: {
                Q_ASSERT(date);
                QDateTime qtDate(QDateTime::fromNSDate(date));
                emit initialized(true, Controller::StateOn, qtDate);
                return;
            }
            case ConnectionStateDisconnected:
                emit initialized(true, Controller::StateOff, QDateTime());
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

void MacOSController::activate(const Server &server,
                               const Device *device,
                               const Keys *keys,
                               const CaptivePortal &captivePortal,
                               bool forSwitching)
{
    Q_UNUSED(device);
    Q_UNUSED(keys);
    Q_UNUSED(captivePortal);
    Q_UNUSED(forSwitching);

    logger.log() << "MacOSController activating" << server.hostname();

    Q_ASSERT(impl);

    const QStringList& captivePortalIpv4Addresses = captivePortal.ipv4Addresses();
    NSMutableArray<NSString *> *captivePortalIpv4AddressesNS = [NSMutableArray<NSString *> arrayWithCapacity:captivePortalIpv4Addresses.length()];
    for (const QString &ip : captivePortalIpv4Addresses) {
        [captivePortalIpv4AddressesNS addObject:ip.toNSString()];
    }

    const QStringList& captivePortalIpv6Addresses = captivePortal.ipv6Addresses();
    NSMutableArray<NSString *> *captivePortalIpv6AddressesNS = [NSMutableArray<NSString *> arrayWithCapacity:captivePortalIpv6Addresses.length()];
    for (const QString &ip : captivePortalIpv6Addresses) {
        [captivePortalIpv6AddressesNS addObject:ip.toNSString()];
    }

    [impl
        connectWithServerIpv4Gateway:server.ipv4Gateway().toNSString()
                   serverIpv6Gateway:server.ipv6Gateway().toNSString()
                     serverPublicKey:server.publicKey().toNSString()
                    serverIpv4AddrIn:server.ipv4AddrIn().toNSString()
                          serverPort:server.choosePort()
          captivePortalIpv4Addresses:captivePortalIpv4AddressesNS
          captivePortalIpv6Addresses:captivePortalIpv6AddressesNS
                         ipv6Enabled:MozillaVPN::instance()->settingsHolder()->ipv6Enabled()
                  localNetworkAccess:MozillaVPN::instance()->settingsHolder()->localNetworkAccess()
                     failureCallback:^() {
                         logger.log() << "MacOSSWiftController - connection failed";
                         emit disconnected();
                     }];
}

void MacOSController::deactivate(bool forSwitching)
{
    Q_UNUSED(forSwitching);

    logger.log() << "MacOSController deactivated";

    Q_ASSERT(impl);
    [impl disconnect];
}

void MacOSController::checkStatus()
{
    logger.log() << "Checking status";

    if (m_checkingStatus) {
        logger.log() << "We are still waiting for the previous status.";
        return;
    }

    m_checkingStatus = true;

    [impl checkStatusWithCallback:^(NSString *serverIpv4Gateway, NSString *configString) {
        QString config = QString::fromNSString(configString);

        m_checkingStatus = false;

        if (config.isEmpty()) {
            return;
        }

        uint64_t txBytes = 0;
        uint64_t rxBytes = 0;

        QStringList lines = config.split("\n");
        for (QStringList::ConstIterator i = lines.begin(); i != lines.end(); ++i) {
            if (i->startsWith("tx_bytes=")) {
                txBytes = i->split("=")[1].toULongLong();
            } else if (i->startsWith("rx_bytes=")) {
                rxBytes = i->split("=")[1].toULongLong();
            }

            if (txBytes && rxBytes) {
                break;
            }
        }

        logger.log() << "ServerIpv4Gateway:" << QString::fromNSString(serverIpv4Gateway)
                     << "RxBytes:" << rxBytes
                     << "TxBytes:" << txBytes;
        emit statusUpdated(QString::fromNSString(serverIpv4Gateway), txBytes, rxBytes);
    }];
}

void MacOSController::getBackendLogs(std::function<void(const QString &)> &&a_callback)
{
    std::function<void(const QString &)> callback = std::move(a_callback);

    QString groupId(GROUP_ID);
    NSURL *groupPath = [[NSFileManager defaultManager] containerURLForSecurityApplicationGroupIdentifier:groupId.toNSString()];

    NSURL *path = [groupPath URLByAppendingPathComponent:@"networkextension.log"];

    QFile file(QString::fromNSString([path path]));
    if (!file.open(QIODevice::ReadOnly)) {
        callback("Network extension log file missing or unreadable.");
        return;
    }

    QByteArray content = file.readAll();
    callback(content);
}
