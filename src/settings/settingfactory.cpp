/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settingfactory.h"

// static
Setting* SettingFactory::createOrGetSetting(
    const QString& key, std::function<QVariant()> defaultValue,
    bool removeWhenReset, bool sensitiveSetting) {
  auto setting = SettingsManager::getSetting(key);
  if (setting {
    Q_ASSERT(defaultValue() == setting->m_defaultValue());
    Q_ASSERT(removeWhenReset == setting->m_removeWhenReset);
    Q_ASSERT(sensitiveSetting == setting->m_sensitiveSetting);

    return s;
  }

  setting = new Setting(
      SettingsManager::instance(), key,
      [defaultValue]() { return QVariant(defaultValue()); }, removeWhenReset,
      sensitiveSetting);

  SettingsManager::instance()->registerSetting(setting);
  return setting;
}
