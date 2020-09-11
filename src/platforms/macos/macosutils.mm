#include "macosutils.h"
#include "keys.h"
#include "device.h"
#include "MozillaVPN-Swift.h"

#import <Cocoa/Cocoa.h>

#include <QDebug>
#include <QByteArray>

// Our Swift singleton.
static MacOSControllerImpl* impl = nullptr;

// static
QString MacosUtils::computerName()
{
    NSString *name = [[NSHost currentHost] localizedName];
    return QString::fromNSString(name);
}

// static
void MacosUtils::controllerActivate(std::function<void(bool)> && a_callback)
{
    Q_ASSERT(impl);

    std::function<void(bool)> callback = std::move(a_callback);

    [impl connectWithClosure:^(BOOL status) {
        callback(status);
    }];
}

// static
void MacosUtils::maybeInitializeController(const Device* device, const Keys* keys, std::function<void(bool)>&& a_callback)
{
    std::function<void(bool)> callback = std::move(a_callback);

    if (impl) {
        callback(true);
        return;
    }

    qDebug() << "Initializing Swift Controller";

    static bool creating = false;
    // No nested creation!
    Q_ASSERT(creating == false);
    creating = true;

    QByteArray key = QByteArray::fromBase64(keys->privateKey().toLocal8Bit());

    impl = [[MacOSControllerImpl alloc]
            initWithPrivateKey:key.toNSData() ipv4Address:device->ipv4Address().toNSString() ipv6Address:device->ipv6Address().toNSString() closure:^(BOOL status) {
        qDebug() << "Creation completed with status" << status;
        creating = false;
        callback(status);
    }];
}
