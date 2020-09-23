#include "macosutils.h"

#include <QDebug>

#import <Cocoa/Cocoa.h>
#import <ServiceManagement/ServiceManagement.h>

// static
QString MacOSUtils::computerName()
{
    NSString *name = [[NSHost currentHost] localizedName];
    return QString::fromNSString(name);
}

// static
void MacOSUtils::enableLoginItem()
{
    qDebug() << "Enabling login-item";
    Boolean ok = SMLoginItemSetEnabled(CFSTR("org.mozilla.macos.FirefoxVPN.login-item"), YES);
    qDebug() << "Result: " << ok;
}
