/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SETTINGSHOLDER_H
#define SETTINGSHOLDER_H

#include <QDateTime>
#include <QObject>
#include <QVariant>

#include "constants.h"
#include "feature.h"
#include "settings/setting.h"
#include "settings/settingfactory.h"
#include "settings/settinggroup.h"

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

 private:
#define SETTING(type, toType, getter, setter, remover, has, key, defaultValue, \
                removeWhenReset, isSensitive)                                  \
  Setting* m_##getter = SettingFactory::createOrGetSetting(                    \
      key, []() -> type { return defaultValue; }, removeWhenReset,             \
      isSensitive);

#include "settingslist.h"
#undef SETTING

  bool m_firstExecution = false;
};

#endif  // SETTINGSHOLDER_H
