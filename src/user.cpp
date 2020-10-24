/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "user.h"
#include "settingsholder.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

bool User::fromJson(const QByteArray &json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json);

    if (!doc.isObject()) {
        return false;
    }

    QJsonObject obj = doc.object();

#define STRSETTER(what, where) \
    { \
        QJsonValue value = obj.take(what); \
        if (!value.isString()) { \
            return false; \
        } \
        where = value.toString(); \
    }

    STRSETTER("avatar", m_avatar);
    STRSETTER("display_name", m_displayName);
    STRSETTER("email", m_email);
#undef STRSETTER

    QJsonValue maxDevices = obj.take("max_devices");
    if (!maxDevices.isDouble()) {
        return false;
    }
    m_maxDevices = maxDevices.toInt();

    QJsonValue subscriptions = obj.take("subscriptions");
    if (!subscriptions.isObject()) {
        return false;
    }

    m_subscriptionNeeded = true;
    QJsonObject subscriptionsObj = subscriptions.toObject();
    if (subscriptionsObj.contains("vpn")) {
        QJsonValue subVpn = subscriptionsObj.take("vpn");
        if (!subVpn.isObject()) {
            return false;
        }

        QJsonObject subVpnObj = subVpn.toObject();
        QJsonValue active = subVpnObj.take("active");
        if (!active.isBool()) {
            return false;
        }

        m_subscriptionNeeded = !active.toBool();
    }

    emit changed();
    return true;
}

bool User::fromSettings(SettingsHolder &settingsHolder)
{
    if (!settingsHolder.hasUserAvatar() || !settingsHolder.hasUserDisplayName()
        || !settingsHolder.hasUserEmail() || !settingsHolder.hasUserMaxDevices()
        || !settingsHolder.hasUserSubscriptionNeeded()) {
        return false;
    }

    m_avatar = settingsHolder.userAvatar();
    m_displayName = settingsHolder.userDisplayName();
    m_email = settingsHolder.userEmail();
    m_maxDevices = settingsHolder.userMaxDevices();
    m_subscriptionNeeded = settingsHolder.userSubscriptionNeeded();

    return true;
}

void User::writeSettings(SettingsHolder &settingsHolder)
{
    settingsHolder.setUserAvatar(m_avatar);
    settingsHolder.setUserDisplayName(m_displayName);
    settingsHolder.setUserEmail(m_email);
    settingsHolder.setUserMaxDevices(m_maxDevices);
    settingsHolder.setUserSubscriptionNeeded(m_subscriptionNeeded);
}
