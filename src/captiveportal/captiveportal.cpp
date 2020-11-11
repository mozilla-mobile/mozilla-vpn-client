/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportal.h"
#include "logger.h"
#include "settingsholder.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace {
Logger logger(LOG_CAPTIVEPORTAL, "CaptivePortal");
}

bool CaptivePortal::fromJson(const QByteArray &data)
{
    logger.log() << "Captive portal from Json";

    QJsonDocument json = QJsonDocument::fromJson(data);
    if (!json.isArray()) {
        logger.log() << "Invalid JSON array";
        return false;
    }

    QStringList ipv4Addresses;
    QStringList ipv6Addresses;

    QJsonArray array = json.array();
    for (QJsonValue entry : array) {
        if (!entry.isObject()) {
            logger.log() << "Object expected";
            return false;
        }

        QJsonObject obj = entry.toObject();
        QJsonValue address = obj.take("address");
        if (!address.isString()) {
            logger.log() << "Address must be a string";
            return false;
        }

        QJsonValue family = obj.take("family");
        if (!family.isDouble()) {
            logger.log() << "Family must be a number";
            return false;
        }

        switch (family.toInt()) {
        case 4:
            ipv4Addresses.append(address.toString());
            break;
        case 6:
            ipv6Addresses.append(address.toString());
            break;
        default:
            logger.log() << "Invalid family number" << family.toInt();
            return false;
        }
    }

    m_ipv4Addresses = ipv4Addresses;
    m_ipv6Addresses = ipv6Addresses;
    return true;
}

bool CaptivePortal::fromSettings()
{
    SettingsHolder *settingsHolder = SettingsHolder::instance();
    Q_ASSERT(settingsHolder);

    if (settingsHolder->hasCaptivePortalIpv4Addresses()) {
        m_ipv4Addresses = settingsHolder->captivePortalIpv4Addresses();
    }
    if (settingsHolder->hasCaptivePortalIpv6Addresses()) {
        m_ipv6Addresses = settingsHolder->captivePortalIpv6Addresses();
    }

    return true;
}

void CaptivePortal::writeSettings()
{
    SettingsHolder *settingsHolder = SettingsHolder::instance();
    Q_ASSERT(settingsHolder);

    settingsHolder->setCaptivePortalIpv4Addresses(m_ipv4Addresses);
    settingsHolder->setCaptivePortalIpv6Addresses(m_ipv6Addresses);
}
