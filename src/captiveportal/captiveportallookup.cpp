/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportallookup.h"
#include "dohrequest.h"
#include "logger.h"
#include "settingsholder.h"

namespace {
Logger logger(LOG_CAPTIVEPORTAL, "CaptivePortalLookup");
}

CaptivePortalLookup::CaptivePortalLookup(QObject *parent) : QObject(parent)
{
    SettingsHolder *settingsHolder = SettingsHolder::instance();
    Q_ASSERT(settingsHolder);

    if (settingsHolder->hasCaptivePortalIpv4Addresses()) {
        m_data.setIpv4Addresses(settingsHolder->captivePortalIpv4Addresses());
    }

    if (settingsHolder->hasCaptivePortalIpv6Addresses()) {
        m_data.setIpv6Addresses(settingsHolder->captivePortalIpv6Addresses());
    }

    connect(&m_timer, &QTimer::timeout, this, &CaptivePortalLookup::timeout);
}

void CaptivePortalLookup::start()
{
    logger.log() << "Captive portal lookup started";
    m_timer.start(CAPTIVEPORTAL_LOOKUPTIMER);

    DOHRequest *request = new DOHRequest(this);
    connect(request,
            &DOHRequest::completed,
            [this](const QStringList &ipv4Addresses, const QStringList &ipv6Addresses) {
                logger.log() << "Lookup completed - ipv4:" << ipv4Addresses
                             << "ipv6:" << ipv6Addresses;
                m_data.setIpv4Addresses(ipv4Addresses);
                m_data.setIpv6Addresses(ipv6Addresses);
                complete();
            });

    request->resolve(CAPTIVEPORTAL_HOST);
}

void CaptivePortalLookup::timeout()
{
    logger.log() << "Lookup timeout! Let's abort the lookup.";
    complete();
}

void CaptivePortalLookup::complete()
{
    m_completed = true;

    SettingsHolder *settingsHolder = SettingsHolder::instance();
    Q_ASSERT(settingsHolder);

    settingsHolder->setCaptivePortalIpv4Addresses(m_data.ipv4Addresses());
    settingsHolder->setCaptivePortalIpv6Addresses(m_data.ipv6Addresses());

    emit completed(m_data);
    deleteLater();
}
