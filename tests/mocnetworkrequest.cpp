/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../src/mozillavpn.h"
#include "helper.h"
#include "networkrequest.h"

namespace {};

NetworkRequest::NetworkRequest(QObject *parent) : QObject(parent)
{
    QTimer::singleShot(0, [this]() {
        deleteLater();
        if (TestHelper::networkStatus == TestHelper::Failure) {
            emit requestFailed(QNetworkReply::NetworkError::HostNotFoundError);
        } else {
            emit requestCompleted(TestHelper::networkBody);
        }
    });
}

// static
NetworkRequest *NetworkRequest::createForAuthenticationVerification(MozillaVPN *vpn,
                                                                    const QString &,
                                                                    const QString &)
{
    return new NetworkRequest(vpn);
}

// static
NetworkRequest *NetworkRequest::createForDeviceCreation(MozillaVPN *vpn,
                                                        const QString &,
                                                        const QString &)
{
    return new NetworkRequest(vpn);
}

// static
NetworkRequest *NetworkRequest::createForDeviceRemoval(MozillaVPN *vpn, const QString &)
{
    return new NetworkRequest(vpn);
}

NetworkRequest *NetworkRequest::createForServers(MozillaVPN *vpn)
{
    return new NetworkRequest(vpn);
}

NetworkRequest *NetworkRequest::createForVersions(MozillaVPN *vpn)
{
    return new NetworkRequest(vpn);
}

NetworkRequest *NetworkRequest::createForAccount(MozillaVPN *vpn)
{
    return new NetworkRequest(vpn);
}

NetworkRequest *NetworkRequest::createForIpInfo(MozillaVPN *vpn)
{
    return new NetworkRequest(vpn);
}

NetworkRequest *NetworkRequest::createForCaptivePortalDetection(QObject *parent)
{
    return new NetworkRequest(parent);
}

#ifdef IOS_INTEGRATION
NetworkRequest *NetworkRequest::createForIOSProducts(MozillaVPN *vpn)
{
    return new NetworkRequest(vpn);
}

NetworkRequest *NetworkRequest::createForIOSPurchase(MozillaVPN *vpn, const QString &)
{
    return new NetworkRequest(vpn);
}
#endif

void NetworkRequest::replyFinished(QNetworkReply *)
{
    QFAIL("Not called!");
}
