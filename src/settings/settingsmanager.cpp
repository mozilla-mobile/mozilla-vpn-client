/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settingsmanager.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>

#ifndef MZ_WASM
#  include "cryptosettings.h"
#endif
#include "leakdetector.h"
#include "logger.h"

namespace {

Logger logger("SettingsManager");

SettingsManager* s_instance = nullptr;

#if defined UNIT_TEST
constexpr const char* SETTINGS_APP_NAME = "vpn_unit";
#else
constexpr const char* SETTINGS_APP_NAME = "vpn";
#endif
}  // namespace

// static
SettingsManager* SettingsManager::instance() {
  if (!s_instance) {
    s_instance = new SettingsManager(qApp);
    qAddPostRoutine([]() { delete s_instance; });
  }

  return s_instance;
}

// static
QString SettingsManager::getOrganizationName() {
  QString name = QApplication::organizationName().toLower();
  // Replace all non-alphanumeric chars with underscores.
  for (qsizetype i = 0; i < name.size(); i++) {
    QChar c = name.at(i);
    if (!c.isLetterOrNumber()) {
      name[i] = '_';
    }
  }
  return name;
}

// static
QSettings::Format SettingsManager::getFormat() {
#ifndef MZ_WASM
  return CryptoSettings::format();
#else
  return QSettings::NativeFormat;
#endif
}

#ifdef UNIT_TEST
// static
void SettingsManager::testCleanup() {
  if (s_instance) {
    delete s_instance;
  }
}
#endif

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent),
      m_settings(getFormat(), QSettings::UserScope, getOrganizationName(),
                 SETTINGS_APP_NAME),
      m_settingsConnector(this, &m_settings) {
  MZ_COUNT_CTOR(SettingsManager);

  logger.debug() << "Initializing SettingsManager";

  LogHandler::instance()->registerLogSerializer(this);
}

SettingsManager::~SettingsManager() {
  MZ_COUNT_DTOR(SettingsManager);

  logger.debug() << "Destroying SettingsManager";

  LogHandler::instance()->unregisterLogSerializer(this);

#ifdef UNIT_TEST
  hardReset();
#endif

  // Free the memory for everything that was in the map.
  qDeleteAll(m_registeredSettings);
  m_registeredSettings.clear();

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

QString SettingsManager::settingsFileName() { return m_settings.fileName(); }

void SettingsManager::registerSetting(Setting* setting) {
  Q_ASSERT(setting);

  if (m_registeredSettings.contains(setting->key())) {
    return;
  }

  m_registeredSettings.insert(setting->key(), setting);
}

void SettingsManager::reset() {
  logger.debug() << "Clean up the settings";
  foreach (Setting* setting, m_registeredSettings.values()) {
    setting->reset();
  }
}

void SettingsManager::hardReset() {
  logger.debug() << "Hard reset";
  m_settings.clear();

  foreach (Setting* setting, m_registeredSettings.values()) {
    Q_ASSERT(setting);
    setting->changed();
  }
}

void SettingsManager::serializeLogs(
    std::function<void(const QString& name, const QString& logs)>&&
        a_callback) {
  std::function<void(const QString& name, const QString& logs)> callback =
      std::move(a_callback);

  QString buff;
  QTextStream out(&buff);
  foreach (Setting* setting, m_registeredSettings.values()) {
    const auto log = setting->log();
    if (!log.isEmpty()) {
      out << log << Qt::endl;
    }
  }

  callback("Settings", buff);
}

Setting* SettingsManager::createOrGetSetting(
    const QString& key, std::function<QVariant()> defaultValueGetter,
    bool removeWhenReset, bool sensitiveSetting) {
  auto setting = getSetting(key);
  if (setting) {
    Q_ASSERT(defaultValueGetter() == setting->m_defaultValueGetter());
    Q_ASSERT(removeWhenReset == setting->m_removeWhenReset);
    Q_ASSERT(sensitiveSetting == setting->m_sensitiveSetting);

    return setting;
  }

  setting = new Setting(
      this, &m_settingsConnector, key,
      [defaultValueGetter]() { return QVariant(defaultValueGetter()); },
      removeWhenReset, sensitiveSetting);

  registerSetting(setting);
  return setting;
}

SettingGroup* SettingsManager::createSettingGroup(const QString& groupKey,
                                                  bool removeWhenReset,
                                                  bool sensitiveSetting,
                                                  QStringList acceptedKeys) {
  return new SettingGroup(this, &m_settingsConnector, groupKey, removeWhenReset,
                          sensitiveSetting, acceptedKeys);
}
