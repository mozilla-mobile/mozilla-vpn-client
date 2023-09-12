/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settingsholder.h"

#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>

#include "constants.h"
#include "cryptosettings.h"
#include "env.h"
#include "feature.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"

namespace {

Logger logger("SettingsHolder");

SettingsHolder* s_instance = nullptr;

const QSettings::Format MozFormat = QSettings::registerFormat(
    "moz", CryptoSettings::readFile, CryptoSettings::writeFile);

}  // namespace

// static
SettingsHolder* SettingsHolder::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

SettingsHolder::SettingsHolder()
    : m_settings(MozFormat, QSettings::UserScope,
#ifndef UNIT_TEST
                 "mozilla",
#else
                 "mozilla_testing",
#endif
                 Constants::SETTINGS_APP_NAME) {
  MZ_COUNT_CTOR(SettingsHolder);

  // The location changes after the initialization of the app. Let's store the
  // journal file-name in the CTOR to avoid race-conditions.
  m_settingsJournalFileName =
      QDir(
#ifdef MZ_WASM
          // https://wiki.qt.io/Qt_for_WebAssembly#Files_and_local_file_system_access
          "/"
#elif defined(UNIT_TEST)
          QStandardPaths::writableLocation(QStandardPaths::TempLocation)
#else
          QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
#endif
          )
          .filePath(
              QString("%1.moz-journal").arg(m_settings.organizationName()));

  if (QFile::exists(m_settingsJournalFileName)) {
    QString journalSettingFile(m_settingsJournalFileName);
    logger.info() << "journal file exists" << journalSettingFile;

    {
      QSettings journalSettings(journalSettingFile, MozFormat);
      for (const QString& key : journalSettings.allKeys()) {
        m_settings.setValue(key, journalSettings.value(key));
      }
    }

    if (!QFile::remove(journalSettingFile)) {
      logger.warning() << "Unable to remove the journal settings file"
                       << journalSettingFile;
    }

#ifdef UNIT_TEST
    m_recoverFromJournal = true;
#endif

    logger.info() << "Recovering completed";
  }

  Q_ASSERT(!s_instance);
  s_instance = this;

  if (!hasInstallationTime()) {
    m_firstExecution = true;
    setInstallationTime(QDateTime::currentDateTime());
  }

  if (!hasUpdateTime() || !hasInstalledVersion() ||
      installedVersion() != Env::versionString()) {
    setUpdateTime(QDateTime::currentDateTime());
    setInstalledVersion(Env::versionString());
  }

  LogHandler::instance()->registerLogSerializer(this);
}

SettingsHolder::~SettingsHolder() {
  MZ_COUNT_DTOR(SettingsHolder);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;

#ifdef UNIT_TEST
  if (!m_doNotClearOnDTOR) {
    m_settings.clear();
  }
#endif

  LogHandler::instance()->unregisterLogSerializer(this);
}

void SettingsHolder::clear() {
  logger.debug() << "Clean up the settings";

#define SETTING(type, toType, getter, setter, remover, has, key, defvalue, \
                userSettings, removeWhenReset, ...)                        \
  if (removeWhenReset) {                                                   \
    m_settings.remove(key);                                                \
    emit getter##Changed();                                                \
  }

#include "settingslist.h"
#undef SETTING
}

void SettingsHolder::sync() { m_settings.sync(); }

void SettingsHolder::hardReset() {
  logger.debug() << "Hard reset";
  m_settings.clear();

#define SETTING(type, toType, getter, ...) emit getter##Changed();

#include "settingslist.h"
#undef SETTING
}

QString SettingsHolder::settingsFileName() const {
  return m_settings.fileName();
}

QVariant SettingsHolder::rawSetting(const QString& key) const {
  return m_settings.value(key);
}

#ifdef UNIT_TEST
void SettingsHolder::setRawSetting(const QString& key, const QVariant& value) {
  m_settings.setValue(key, value);
}
#endif

#define SETTING(type, toType, getter, setter, remover, has, key, defvalue, \
                userSettings, ...)                                         \
  bool SettingsHolder::has() const { return m_settings.contains(key); }    \
  type SettingsHolder::getter() const {                                    \
    if (!has()) {                                                          \
      return defvalue;                                                     \
    }                                                                      \
    return m_settings.value(key).toType();                                 \
  }                                                                        \
  void SettingsHolder::setter(const type& value) {                         \
    if (!has() || getter() != value) {                                     \
      maybeSaveInTransaction(key, getter(), value, #getter "Changed",      \
                             userSettings);                                \
      m_settings.setValue(key, value);                                     \
      emit getter##Changed();                                              \
    }                                                                      \
  }                                                                        \
  void SettingsHolder::remover() {                                         \
    m_settings.remove(key);                                                \
    emit getter##Changed();                                                \
  }

#include "settingslist.h"
#undef SETTING

// Addon specific

void SettingsHolder::clearAddonSettings(const QString& group) {
  logger.debug() << "Clean up the settings for group" << group;

  const QString groupKey(
      QString("%1/%2").arg(Constants::ADDON_SETTINGS_GROUP, group));

  m_settings.beginGroup(groupKey);
  m_settings.remove("");
  m_settings.endGroup();

  emit addonSettingsChanged();
}

// static
QString SettingsHolder::getAddonSettingKey(const AddonSettingQuery& query) {
  return QString("%1/%2/%3/%4")
      .arg(Constants::ADDON_SETTINGS_GROUP, query.m_addonGroup, query.m_addonId,
           query.m_setting);
}

QString SettingsHolder::getAddonSetting(const AddonSettingQuery& query) {
  QString key = getAddonSettingKey(query);

  if (!m_settings.contains(key)) return query.m_defaultValue;

  return m_settings.value(key).toString();
}

void SettingsHolder::setAddonSetting(const AddonSettingQuery& query,
                                     const QString& value) {
  QString key = getAddonSettingKey(query);

  if (m_settings.value(key).toString() != value) {
    m_settings.setValue(key, value);
    emit addonSettingsChanged();
  }
}

bool SettingsHolder::beginTransaction() {
  if (m_settingsJournal) {
    logger.warning() << "Nested transactions are not supported";
    return false;
  }

  sync();

  if (!QFile::copy(m_settings.fileName(), m_settingsJournalFileName)) {
    logger.warning() << "Unable to generate a setting journal file"
                     << m_settingsJournalFileName;
    return false;
  }

  m_settingsJournal =
      new QSettings(m_settingsJournalFileName, m_settings.format(), this);

  emit transactionBegan();
  return true;
}

bool SettingsHolder::commitTransaction() {
  if (!m_settingsJournal) {
    logger.warning() << "We are not in a transaction";
    return false;
  }

  return finalizeTransaction();
}

bool SettingsHolder::rollbackTransaction() {
  if (!m_settingsJournal) {
    logger.warning() << "We are not in a transaction";
    return false;
  }

  emit transactionAboutToRollBack();

  QMap<QString, QPair<const char*, QVariant>> transactionChanges(
      m_transactionChanges);

  auto guard = qScopeGuard([&] { emit transactionRolledBack(); });

  if (!finalizeTransaction()) {
    return false;
  }

  QMapIterator<QString, QPair<const char*, QVariant>> i(transactionChanges);
  while (i.hasNext()) {
    i.next();
    m_settings.setValue(i.key(), i.value().second);
    QMetaObject::invokeMethod(this, i.value().first, Qt::DirectConnection);
  }

  return true;
}

bool SettingsHolder::finalizeTransaction() {
  m_transactionChanges.clear();
  delete m_settingsJournal;
  m_settingsJournal = nullptr;

  if (!QFile::remove(m_settingsJournalFileName)) {
    logger.warning() << "Unable to remove the setting journal file"
                     << m_settingsJournalFileName;
    return false;
  }

  return true;
}

void SettingsHolder::maybeSaveInTransaction(const QString& key,
                                            const QVariant& oldValue,
                                            const QVariant& newValue,
                                            const char* signalName,
                                            bool userSettings) {
  if (!m_settingsJournal) {
    return;
  }

  if (!userSettings) {
    m_settingsJournal->setValue(key, newValue);
    return;
  }

  if (!m_transactionChanges.contains(key)) {
    m_transactionChanges.insert(key, {signalName, oldValue});
  }
}

void SettingsHolder::serializeLogs(
    std::function<void(const QString& name, const QString& logs)>&&
        a_callback) {
  std::function<void(const QString& name, const QString& logs)> callback =
      std::move(a_callback);

  QString buff;
  QTextStream out(&buff);
#define SETTING(type, toType, getter, setter, remover, has, key, defvalue, \
                userSettings, removeWhenReset, sensitive)                  \
  if (has()) {                                                             \
    if (sensitive) {                                                       \
      out << key << " -> <Sensitive>" << Qt::endl;                         \
    } else {                                                               \
      out << key << " -> ";                                                \
      QVariant value = m_settings.value(key);                              \
      switch (value.typeId()) {                                            \
        case QVariant::List:                                               \
        case QVariant::StringList:                                         \
          out << '[' << value.toStringList().join(",") << ']' << ' ';      \
          break;                                                           \
        default:                                                           \
          out << value.toString();                                         \
      }                                                                    \
      out << Qt::endl;                                                     \
    }                                                                      \
  }
#include "settingslist.h"
#undef SETTING

  callback("Settings", buff);
}
