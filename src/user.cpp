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

void User::fromJson(const QByteArray &json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json);

    Q_ASSERT(doc.isObject());
    QJsonObject obj = doc.object();

    Q_ASSERT(obj.contains("avatar"));
    QJsonValue avatarValue = obj.take("avatar");
    Q_ASSERT(avatarValue.isString());
    m_avatar = avatarValue.toString();

    Q_ASSERT(obj.contains("display_name"));
    QJsonValue displayName = obj.take("display_name");
    Q_ASSERT(displayName.isString());
    m_displayName = displayName.toString();

    Q_ASSERT(obj.contains("email"));
    QJsonValue email = obj.take("email");
    Q_ASSERT(email.isString());
    m_email = email.toString();

    Q_ASSERT(obj.contains("max_devices"));
    QJsonValue maxDevices = obj.take("max_devices");
    Q_ASSERT(maxDevices.isDouble());
    m_maxDevices = maxDevices.toInt();

    Q_ASSERT(obj.contains("subscriptions"));
    QJsonValue subscriptions = obj.take("subscriptions");
    Q_ASSERT(subscriptions.isObject());

    m_subscriptionNeeded = true;
    QJsonObject subscriptionsObj = subscriptions.toObject();
    if (subscriptionsObj.contains("vpn")) {
        QJsonValue subVpn = subscriptionsObj.take("vpn");
        Q_ASSERT(subVpn.isObject());

        QJsonObject subVpnObj = subVpn.toObject();
        Q_ASSERT(subVpnObj.contains("active"));
        QJsonValue active = subVpnObj.take("active");
        Q_ASSERT(active.isBool());

        m_subscriptionNeeded = !active.toBool();
    }

    emit changed();
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
