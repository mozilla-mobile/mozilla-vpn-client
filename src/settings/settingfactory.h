/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SETTINGSFACTORY_H
#define SETTINGSFACTORY_H

#include "setting.h"
#include "settingsmanager.h"

class SettingFactory {
 public:
  /**
   * @brief Construct a new Setting object and register it with the
   * SettingsManager.
   *
   * If another setting with the same key is already registered, no new setting
   * is registered and a pointer to the existing setting is returned. This
   * function will crash in debug mode in case the existing setting has a
   * different configuration from the new setting.
   *
   * @param key A QSettings valid key.
   * @param defaultValue The default value to be returned when this setting is
   * not set. Default value will be null if unset.
   * @param removeWhenReset Whether or not this setting should actually be
   * removed when `reset` is called. Default is true.
   * @param sensitiveSetting Whether or not this is a sensitive setting i.e. a
   * setting that must not be logged in plain text. Default is false.
   */
  static Setting* createOrGetSetting(
      const QString& key,
      std::function<QVariant()> defaultValue = []() { return QVariant(); },
      bool removeWhenReset = true, bool sensitiveSetting = false);

 private:
  SettingFactory() = default;
  ~SettingFactory() = default;
};

#endif  // SETTINGSFACTORY_H
