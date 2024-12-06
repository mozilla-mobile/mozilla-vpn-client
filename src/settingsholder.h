/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SETTINGSHOLDER_H
#define SETTINGSHOLDER_H

#include <QDateTime>
#include <QObject>
#include <QVariant>

// Used in the ettingslist.h
#include "constants.h"        // IWYU pragma: keep
#include "feature/feature.h"  // IWYU pragma: keep
#include "settings/setting.h"
#include "settings/settingsmanager.h"

constexpr const char* EXPERIMENTS_SETTING_GROUP = "experiments";

/**
 * @brief The SettingsHolder class is a singleton that exposes the APIs to
 * interact with build time declared static settings.
 *
 */
class SettingsHolder final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(SettingsHolder)

 public:
#define SETTING(type, toType, getter, setter, ...)                        \
  Q_PROPERTY(type getter READ getter WRITE setter NOTIFY getter##Changed) \
  Q_SIGNAL void getter##Changed();

#include "settingslist.h"
#undef SETTING

  SettingsHolder();
  ~SettingsHolder();

  static SettingsHolder* instance();

  bool firstExecution() const { return m_firstExecution; }

  enum DNSProviderFlags {
    Gateway = 0x00,
    Custom = 0x01,
    BlockAds = 0x02,
    BlockTrackers = 0x04,
    BlockMalware = 0x08,
  };
  Q_ENUM(DNSProviderFlags)

#define SETTING(type, toType, getter, setter, remover, has, ...) \
  bool has() const { return m_##getter->isSet(); }               \
  type getter() const { return m_##getter->get().toType(); }     \
  void setter(const type& value) { m_##getter->set(value); }     \
  void remover() { return m_##getter->remove(); }

#include "settingslist.h"
#undef SETTING

#define EXPERIMENTAL_FEATURE(experimentId, ...) \
  SettingGroup* experimentId() { return m_##experimentId; }

#include "feature/experimentalfeaturelist.h"
#undef EXPERIMENTAL_FEATURE

 private:
#define SETTING(type, toType, getter, setter, remover, has, key, defaultValue, \
                removeWhenReset, isSensitive)                                  \
  Setting* m_##getter = SettingsManager::instance()->createOrGetSetting(       \
      key, []() -> type { return defaultValue; }, removeWhenReset,             \
      isSensitive);

#include "settingslist.h"
#undef SETTING

#define EXPERIMENTAL_FEATURE(experimentId, experimentDescription,             \
                             experimentSettings)                              \
  SettingGroup* m_##experimentId =                                            \
      SettingsManager::instance()->createSettingGroup(                        \
          QString("%1/%2").arg(EXPERIMENTS_SETTING_GROUP).arg(#experimentId), \
          true, false, experimentSettings);

#include "feature/experimentalfeaturelist.h"
#undef EXPERIMENTAL_FEATURE

  bool m_firstExecution = false;
};

#endif  // SETTINGSHOLDER_H
