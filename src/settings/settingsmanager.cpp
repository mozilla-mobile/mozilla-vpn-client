/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settingsmanager.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>

#include "constants.h"
#include "cryptosettings.h"
#include "leakdetector.h"
#include "logger.h"

namespace {

Logger logger("SettingsManager");

SettingsManager* s_instance = nullptr;

const QSettings::Format MozFormat = QSettings::registerFormat(
    "moz", CryptoSettings::readFile, CryptoSettings::writeFile);

}  // namespace

// static
SettingsManager* SettingsManager::instance() {
  if (!s_instance) {
    s_instance = new SettingsManager(qApp);
  }

  return s_instance;
}

#ifdef UNIT_TEST
// static
void SettingsManager::testCleanup() { delete s_instance; }
#endif

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent),
      m_settings(MozFormat, QSettings::UserScope,
#ifndef UNIT_TEST
                 "mozilla",
#else
                 "mozilla_testing",
#endif
                 Constants::SETTINGS_APP_NAME) {
  MZ_COUNT_CTOR(SettingsManager);

  logger.debug() << "Initializing SettingsManager";

  LogHandler::instance()->registerLogSerializer(this);
}

SettingsManager::~SettingsManager() {
  MZ_COUNT_DTOR(SettingsManager);

  logger.debug() << "Destroying SettingsManager";

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;

  LogHandler::instance()->unregisterLogSerializer(this);

#ifdef UNIT_TEST
  hardReset();
#endif
}

// static
QString SettingsManager::settingsFileName() {
  return instance()->m_settings.fileName();
}

// static
void SettingsManager::registerSetting(Setting* setting) {
  Q_ASSERT(setting);

  if (instance()->m_registeredSettings.contains(setting->key())) {
    return;
  }

  instance()->m_registeredSettings.insert(setting->key(), setting);
}

// static
void SettingsManager::reset() {
  logger.debug() << "Clean up the settings";
  foreach (Setting* setting, instance()->m_registeredSettings.values()) {
    setting->reset();
  }
}

// static
void SettingsManager::hardReset() {
  logger.debug() << "Hard reset";
  instance()->m_settings.clear();

  foreach (Setting* setting, instance()->m_registeredSettings.values()) {
    setting->changed();
  }

  instance()->m_registeredSettings.clear();
  // Free the memory for everything that was in the map.
  qDeleteAll(instance()->m_registeredSettings);
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

// static
Setting* SettingsManager::createOrGetSetting(
    const QString& key, std::function<QVariant()> defaultValue,
    bool removeWhenReset, bool sensitiveSetting) {
  auto setting = getSetting(key);
  if (setting) {
    Q_ASSERT(defaultValue() == setting->m_defaultValue());
    Q_ASSERT(removeWhenReset == setting->m_removeWhenReset);
    Q_ASSERT(sensitiveSetting == setting->m_sensitiveSetting);

    return setting;
  }

  setting = new Setting(
      SettingsManager::instance(), key,
      [defaultValue]() { return QVariant(defaultValue()); }, removeWhenReset,
      sensitiveSetting);

  instance()->registerSetting(setting);
  return setting;
}
