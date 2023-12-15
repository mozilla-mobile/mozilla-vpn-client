/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settingsbase.h"

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

Logger logger("SettingsBase");

SettingsBase* s_instance = nullptr;

const QSettings::Format MozFormat = QSettings::registerFormat(
    "moz", CryptoSettings::readFile, CryptoSettings::writeFile);

}  // namespace

// static
SettingsBase* SettingsBase::instance() {
  if (!s_instance) {
    s_instance = new SettingsBase(qApp);
  }

  return s_instance;
}

#ifdef UNIT_TEST
// static
void SettingsBase::testCleanup() { delete s_instance; }
#endif

SettingsBase::SettingsBase(QObject* parent)
    : QObject(parent),
      m_settings(MozFormat, QSettings::UserScope,
#ifndef UNIT_TEST
                 "mozilla",
#else
                 "mozilla_testing",
#endif
                 Constants::SETTINGS_APP_NAME) {
  MZ_COUNT_CTOR(SettingsBase);

  logger.debug() << "Initializing SettingsBase";

  LogHandler::instance()->registerLogSerializer(this);
}

SettingsBase::~SettingsBase() {
  MZ_COUNT_DTOR(SettingsBase);

  logger.debug() << "Destroying SettingsBase";

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;

  LogHandler::instance()->unregisterLogSerializer(this);

#ifdef UNIT_TEST
  hardReset();
#endif
}

// static
QString SettingsBase::settingsFileName() {
  return instance()->m_settings.fileName();
}

// static
void SettingsBase::registerSetting(Setting* setting) {
  Q_ASSERT(setting);

  if (instance()->m_registeredSettings.contains(setting->key())) {
    return;
  }

  instance()->m_registeredSettings.insert(setting->key(), setting);
}

// static
void SettingsBase::reset() {
  logger.debug() << "Clean up the settings";
  foreach (Setting* setting, instance()->m_registeredSettings.values()) {
    setting->reset();
  }
}

// static
void SettingsBase::hardReset() {
  logger.debug() << "Hard reset";
  instance()->m_settings.clear();

  foreach (Setting* setting, instance()->m_registeredSettings.values()) {
    setting->changed();
  }

  instance()->m_registeredSettings.clear();
  // Free the memory for everythig that was in the map.
  qDeleteAll(instance()->m_registeredSettings);
}

void SettingsBase::serializeLogs(
    std::function<void(const QString& name, const QString& logs)>&&
        a_callback) {
  std::function<void(const QString& name, const QString& logs)> callback =
      std::move(a_callback);

  QString buff;
  QTextStream out(&buff);
  foreach (Setting* setting, m_registeredSettings.values()) {
    auto log = setting->log();
    if (!log.isEmpty()) {
      out << log << Qt::endl;
    }
  }

  callback("Settings", buff);
}
