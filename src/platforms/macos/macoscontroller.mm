#include "macoscontroller.h"
#include "mozillavpn.h"
#include "server.h"
#include "keys.h"
#include "device.h"
#include "Mozilla_VPN-Swift.h"

#import <Cocoa/Cocoa.h>

#include <QDebug>
#include <QByteArray>

// Our Swift singleton.
static MacOSControllerImpl *impl = nullptr;

void MacOSController::initialize(const Device *device, const Keys *keys) {
    Q_ASSERT(!impl);

    qDebug() << "Initializing Swift Controller";

    static bool creating = false;
    // No nested creation!
    Q_ASSERT(creating == false);
    creating = true;

    QByteArray key = QByteArray::fromBase64(keys->privateKey().toLocal8Bit());

    impl = [[MacOSControllerImpl alloc] initWithPrivateKey:key.toNSData()
        ipv4Address:device->ipv4Address().toNSString()
        ipv6Address:device->ipv6Address().toNSString()
        ipv6Enabled:MozillaVPN::instance()->settingsHolder()->ipv6()
        closure:^(ConnectionState state, NSDate *date) {
            qDebug() << "Creation completed with connection state:" << state;
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
            qDebug() << "State changed: " << a_connected;
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
                               bool forSwitching)
{
    Q_UNUSED(device);
    Q_UNUSED(keys);
    Q_UNUSED(forSwitching);

    qDebug() << "MacOSController activating" << server.hostname();

    Q_ASSERT(impl);

    [impl connectWithServerIpv4Gateway:server.ipv4Gateway().toNSString()
                     serverIpv6Gateway:server.ipv6Gateway().toNSString()
                       serverPublicKey:server.publicKey().toNSString()
                      serverIpv4AddrIn:server.ipv4AddrIn().toNSString()
                            serverPort:server.choosePort()
                           ipv6Enabled:MozillaVPN::instance()->settingsHolder()->ipv6()
                       failureCallback:^() {
                           qDebug() << "MacOSSWiftController - connection failed";
                           emit disconnected();
                       }];
}

void MacOSController::deactivate(const Server &server,
                                 const Device *device,
                                 const Keys *keys,
                                 bool forSwitching)
{
    Q_UNUSED(device);
    Q_UNUSED(keys);
    Q_UNUSED(forSwitching);

    qDebug() << "MacOSController deactivated" << server.hostname();

    Q_ASSERT(impl);
    [impl disconnect];
}

void MacOSController::checkStatus()
{
    qDebug() << "Checking status";

    if (m_checkingStatus) {
        qDebug() << "We are still waiting for the previous status.";
        return;
    }

    m_checkingStatus = true;

    [impl checkStatusWithCallback:^(NSString *configString) {
        QString config = QString::fromNSString(configString);

        m_checkingStatus = false;

        if (config.isEmpty()) {
            return;
        }

        uint32_t txBytes = 0;
        uint32_t rxBytes = 0;

        QStringList lines = config.split("\n");
        for (QStringList::ConstIterator i = lines.begin(); i != lines.end(); ++i) {
            if (i->startsWith("tx_bytes=")) {
                txBytes = i->split("=")[1].toUInt();
            } else if (i->startsWith("rx_bytes=")) {
                rxBytes = i->split("=")[1].toUInt();
            }

            if (txBytes && rxBytes) {
                break;
            }
        }

        qDebug() << "RxBytes:" << rxBytes << "TxBytes:" << txBytes;
        emit statusUpdated(txBytes, rxBytes);
    }];
}
