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
NetworkRequest *NetworkRequest::createForAuthenticationVerification(QObject *parent, MozillaVPN *, const QString &, const QString &)
{
    return new NetworkRequest(parent);
}

// static
NetworkRequest *NetworkRequest::createForDeviceCreation(QObject *parent, MozillaVPN *, const QString &, const QString &)
{
    return new NetworkRequest(parent);
}

// static
NetworkRequest *NetworkRequest::createForDeviceRemoval(QObject *parent, MozillaVPN *, const QString &)
{
    return new NetworkRequest(parent);
}

NetworkRequest *NetworkRequest::createForServers(QObject *parent, MozillaVPN *)
{
    return new NetworkRequest(parent);
}

NetworkRequest *NetworkRequest::createForVersions(QObject *parent, MozillaVPN *)
{
    return new NetworkRequest(parent);
}

NetworkRequest *NetworkRequest::createForAccount(QObject *parent, MozillaVPN *)
{
    return new NetworkRequest(parent);
}

NetworkRequest *NetworkRequest::createForIpInfo(QObject *parent, MozillaVPN *)
{
    return new NetworkRequest(parent);
}

NetworkRequest *NetworkRequest::createForCaptivePortalDetection(QObject *parent)
{
    return new NetworkRequest(parent);
}

#ifdef IOS_INTEGRATION
NetworkRequest *NetworkRequest::createForIOSProducts(QObject *parent, MozillaVPN *)
{
    return new NetworkRequest(parent);
}

NetworkRequest *NetworkRequest::createForIOSPurchase(QObject *parent, MozillaVPN *, const QString &)
{
    return new NetworkRequest(parent);
}
#endif

void NetworkRequest::replyFinished(QNetworkReply *)
{
    QFAIL("Not called!");
}
