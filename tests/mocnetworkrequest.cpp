/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../src/timersingleshot.h"
#include "helper.h"
#include "networkrequest.h"

namespace {};

NetworkRequest::NetworkRequest(QObject *parent) : QObject(parent)
{
    Q_ASSERT(!TestHelper::networkConfig.isEmpty());
    TestHelper::NetworkConfig nc = TestHelper::networkConfig.takeFirst();

    TimerSingleShot::create(this, 0, [this, nc]() {
        deleteLater();
        if (nc.m_status == TestHelper::NetworkConfig::Failure) {
            emit requestFailed(QNetworkReply::NetworkError::HostNotFoundError);
        } else {
            Q_ASSERT(nc.m_status == TestHelper::NetworkConfig::Success);
            emit requestCompleted(nc.m_body);
        }
    });
}

// static
NetworkRequest *NetworkRequest::createForAuthenticationVerification(QObject *parent, const QString &, const QString &)
{
    return new NetworkRequest(parent);
}

// static
NetworkRequest *NetworkRequest::createForDeviceCreation(QObject *parent, const QString &, const QString &)
{
    return new NetworkRequest(parent);
}

// static
NetworkRequest *NetworkRequest::createForDeviceRemoval(QObject *parent, const QString &)
{
    return new NetworkRequest(parent);
}

NetworkRequest *NetworkRequest::createForServers(QObject *parent)
{
    return new NetworkRequest(parent);
}

NetworkRequest *NetworkRequest::createForVersions(QObject *parent)
{
    return new NetworkRequest(parent);
}

NetworkRequest *NetworkRequest::createForAccount(QObject *parent)
{
    return new NetworkRequest(parent);
}

NetworkRequest *NetworkRequest::createForIpInfo(QObject *parent)
{
    return new NetworkRequest(parent);
}

NetworkRequest *NetworkRequest::createForCaptivePortalDetection(QObject *parent,
                                                                const QUrl &,
                                                                const QByteArray &)
{
    return new NetworkRequest(parent);
}

NetworkRequest *NetworkRequest::createForCaptivePortalLookup(QObject *parent)
{
    return new NetworkRequest(parent);
}

#ifdef IOS_INTEGRATION
NetworkRequest *NetworkRequest::createForIOSProducts(QObject *parent)
{
    return new NetworkRequest(parent);
}

NetworkRequest *NetworkRequest::createForIOSPurchase(QObject *parent, const QString &)
{
    return new NetworkRequest(parent);
}
#endif

void NetworkRequest::replyFinished()
{
    QFAIL("Not called!");
}
