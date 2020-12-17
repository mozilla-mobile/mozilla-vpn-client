/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "user.h"
#include "leakdetector.h"
#include "settingsholder.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

User::User() { MVPN_COUNT_CTOR(User); }

User::~User() { MVPN_COUNT_DTOR(User); }

bool User::fromJson(const QByteArray& json) {
  m_initialized = false;

  QJsonDocument doc = QJsonDocument::fromJson(json);

  if (!doc.isObject()) {
    return false;
  }

  QJsonObject obj = doc.object();

  QJsonValue avatar = obj.value("avatar");
  if (!avatar.isString()) {
    return false;
  }

  QJsonValue displayName = obj.value("display_name");
  if (!displayName.isString()) {
    return false;
  }

  QJsonValue email = obj.value("email");
  if (!email.isString()) {
    return false;
  }

  QJsonValue maxDevices = obj.value("max_devices");
  if (!maxDevices.isDouble()) {
    return false;
  }

  QJsonValue subscriptions = obj.value("subscriptions");
  if (!subscriptions.isObject()) {
    return false;
  }

  bool subscriptionNeeded = true;
  QJsonObject subscriptionsObj = subscriptions.toObject();
  if (subscriptionsObj.contains("vpn")) {
    QJsonValue subVpn = subscriptionsObj.value("vpn");
    if (!subVpn.isObject()) {
      return false;
    }

    QJsonObject subVpnObj = subVpn.toObject();
    QJsonValue active = subVpnObj.value("active");
    if (!active.isBool()) {
      return false;
    }

    subscriptionNeeded = !active.toBool();
  }

  m_avatar = avatar.toString();
  m_displayName = displayName.toString();
  m_email = email.toString();
  m_maxDevices = maxDevices.toInt();
  m_subscriptionNeeded = subscriptionNeeded;
  m_initialized = true;

  emit changed();
  return true;
}

bool User::fromSettings() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  if (!settingsHolder->hasUserAvatar() ||
      !settingsHolder->hasUserDisplayName() ||
      !settingsHolder->hasUserEmail() || !settingsHolder->hasUserMaxDevices() ||
      !settingsHolder->hasUserSubscriptionNeeded()) {
    return false;
  }

  m_avatar = settingsHolder->userAvatar();
  m_displayName = settingsHolder->userDisplayName();
  m_email = settingsHolder->userEmail();
  m_maxDevices = settingsHolder->userMaxDevices();
  m_subscriptionNeeded = settingsHolder->userSubscriptionNeeded();
  m_initialized = true;

  return true;
}

void User::writeSettings() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  settingsHolder->setUserAvatar(m_avatar);
  settingsHolder->setUserDisplayName(m_displayName);
  settingsHolder->setUserEmail(m_email);
  settingsHolder->setUserMaxDevices(m_maxDevices);
  settingsHolder->setUserSubscriptionNeeded(m_subscriptionNeeded);
}
