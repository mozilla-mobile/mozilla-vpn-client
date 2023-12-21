/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settings/settinggroup.h"

#include "leakdetector.h"
#include "settingsmanager.h"

SettingGroup::SettingGroup(QObject* parent,
                           SettingsConnector* settingsConnector,
                           const QString& groupKey, bool removeWhenReset,
                           bool sensitiveSetting, QStringList acceptedKeys)
    : QObject(parent),
      m_groupKey(groupKey),
      m_sensitiveSetting(sensitiveSetting),
      m_removeWhenReset(removeWhenReset),
      m_acceptedKeys(acceptedKeys),
      m_settingsConnector(settingsConnector) {
  MZ_COUNT_CTOR(Setting);

  // Group settings are dynamic, therefore we need to load from memory all
  // settings that exist under this group prefix in order to emit change signals
  // when they change.
  foreach (const QString& key, m_settingsConnector->getAllKeys(m_groupKey)) {
    addSetting(key);
  }
}

SettingGroup::~SettingGroup() { MZ_COUNT_DTOR(Setting); }

void SettingGroup::addSetting(const QString& key) {
  auto settingKey = buildSettingKeyWithGroupPrefix(key);
  auto setting = SettingsManager::instance()->createOrGetSetting(
      settingKey, []() { return QVariant(); }, m_removeWhenReset,
      m_sensitiveSetting);

  // Emit the group change signal for this group when the setting is changed.
  connect(setting, &Setting::changed, this, [this]() { emit changed(); });
}

bool SettingGroup::isAcceptedKey(const QString& key) {
  if (m_acceptedKeys.count() == 0) {
    return true;
  }

  return m_acceptedKeys.contains(key);
}

QVariant SettingGroup::get(const QString& key) const {
  auto setting = SettingsManager::instance()->getSetting(
      buildSettingKeyWithGroupPrefix(key));
  if (!setting) {
    return QVariant();
  }

  return setting->get();
}

void SettingGroup::set(const QString& key, QVariant value) {
  if (!isAcceptedKey(key)) {
    return;
  }

  auto fullKey = buildSettingKeyWithGroupPrefix(key);
  auto setting = SettingsManager::instance()->getSetting(fullKey);
  if (!setting) {
    addSetting(key);
    setting = SettingsManager::instance()->getSetting(fullKey);
  }

  setting->set(value);
}

void SettingGroup::remove() {
  QStringList keys = m_settingsConnector->getAllKeys(m_groupKey);

  m_settingsConnector->remove("", m_groupKey);

  foreach (const QString& key, keys) {
    auto fullKey = buildSettingKeyWithGroupPrefix(key);
    auto setting = SettingsManager::instance()->getSetting(fullKey);
    setting->changed();
  }
}
