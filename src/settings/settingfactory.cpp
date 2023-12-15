/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settingfactory.h"

// static
Setting* SettingFactory::createOrGetSetting(
    const QString& key, std::function<QVariant()> defaultValue,
    bool removeWhenReset, bool sensitiveSetting) {
  auto s = SettingsBase::getSetting(key);
  if (s) {
    Q_ASSERT(defaultValue() == s->m_defaultValue());
    Q_ASSERT(removeWhenReset == s->m_removeWhenReset);
    Q_ASSERT(sensitiveSetting == s->m_sensitiveSetting);

    return s;
  }

  s = new Setting(
      SettingsBase::instance(), key,
      [defaultValue]() { return QVariant(defaultValue()); }, removeWhenReset,
      sensitiveSetting);

  SettingsBase::instance()->registerSetting(s);
  return s;
}

// static
Setting* SettingFactory::createOrGetSetting(
    const QString& key, std::function<std::nullptr_t()> defaultValue,
    bool removeWhenReset, bool sensitiveSetting) {
  return createOrGetSetting(
      key, []() { return QVariant(); }, removeWhenReset, sensitiveSetting);
}
