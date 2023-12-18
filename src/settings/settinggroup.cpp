/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settings/settinggroup.h"

#include "leakdetector.h"
#include "settingfactory.h"
#include "settingsmanager.h"

SettingGroup::SettingGroup(const QString& groupKey, bool removeWhenReset,
                           bool sensitiveSetting, QStringList acceptedKeys)
    : m_groupKey(groupKey),
      m_sensitiveSetting(sensitiveSetting),
      m_removeWhenReset(removeWhenReset),
      m_acceptedKeys(acceptedKeys) {
  MZ_COUNT_CTOR(Setting);

  // Group settings are dynamic, therefore we need to load from memory all
  // settings that exist under this group prefix in order to emit change signals
  // when they change.
  auto sb = SettingsManager::instance();
  sb->m_settings.beginGroup(m_groupKey);
  QStringList keys = sb->m_settings.allKeys();
  sb->m_settings.endGroup();
  foreach (const QString& key, keys) {
    addSetting(key);
  }
}

SettingGroup::~SettingGroup() { MZ_COUNT_DTOR(Setting); }

void SettingGroup::addSetting(const QString& key) {
  auto settingKey = getSettingKey(key);
  auto setting = SettingFactory::createOrGetSetting(
      settingKey, []() { return nullptr; }, m_removeWhenReset,
      m_sensitiveSetting);

  // Emit the group change signal for this group when the setting is changed.
  connect(setting, &Setting::changed, [key, this]() { emit changed(); });
}

bool SettingGroup::mayRecord(const QString& key) {
  if (m_acceptedKeys.count() == 0) {
    return true;
  }

  return m_acceptedKeys.contains(key);
}

QVariant SettingGroup::get(const QString& key) const {
  auto setting = SettingsManager::getSetting(getSettingKey(key));
  if (!setting) {
    return QVariant();
  }

  return setting->get();
}

void SettingGroup::set(const QString& key, QVariant value) {
  if (!mayRecord(key)) {
    return;
  }

  auto fullKey = getSettingKey(key);
  auto setting = SettingsManager::getSetting(fullKey);
  if (!setting) {
    addSetting(key);
    setting = SettingsManager::getSetting(fullKey);
  }

  setting->set(value);
}

void SettingGroup::remove() {
  auto sb = SettingsManager::instance();
  sb->m_settings.beginGroup(m_groupKey);
  QStringList keys = sb->m_settings.allKeys();
  sb->m_settings.remove("");
  sb->m_settings.endGroup();

  foreach (const QString& key, keys) {
    auto setting = SettingsManager::getSetting(key);
    Q_ASSERT(setting);
    setting->changed();
  }
}
