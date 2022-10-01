/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settingsholder.h"
#include "constants.h"
#include "cryptosettings.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"

#include <QSettings>

namespace {

Logger logger(LOG_MAIN, "SettingsHolder");
// Setting Keys That won't show up in a report;
QVector<QString> SENSITIVE_SETTINGS({
    "token", "privateKey",
    "servers",  // Those 2 are not sensitive but
    "devices",  // are more noise then info
});

SettingsHolder* s_instance = nullptr;

}  // namespace

// static
SettingsHolder* SettingsHolder::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

#ifndef UNIT_TEST
const QSettings::Format MozFormat = QSettings::registerFormat(
    "moz", CryptoSettings::readFile, CryptoSettings::writeFile);
#endif

SettingsHolder::SettingsHolder()
    :
#ifndef UNIT_TEST
      m_settings(MozFormat, QSettings::UserScope, "mozilla", "vpn")
#else
      m_settings("mozilla_testing", "vpn")
#endif
{
  MVPN_COUNT_CTOR(SettingsHolder);

  Q_ASSERT(!s_instance);
  s_instance = this;

  if (!hasInstallationTime()) {
    m_firstExecution = true;
    setInstallationTime(QDateTime::currentDateTime());
  }
}

SettingsHolder::~SettingsHolder() {
  MVPN_COUNT_DTOR(SettingsHolder);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;

#ifdef UNIT_TEST
  m_settings.clear();
#endif
}

void SettingsHolder::clear() {
  logger.debug() << "Clean up the settings";

#define SETTING(type, toType, getter, setter, has, key, defvalue, \
                removeWhenReset)                                  \
  if (removeWhenReset) {                                          \
    m_settings.remove(key);                                       \
    emit getter##Changed(defvalue);                               \
  }

#include "settingslist.h"
#undef SETTING
}

void SettingsHolder::sync() { m_settings.sync(); }

void SettingsHolder::hardReset() {
  logger.debug() << "Hard reset";
  m_settings.clear();

#define SETTING(type, toType, getter, setter, has, key, defvalue, \
                removeWhenReset)                                  \
  emit getter##Changed(defvalue);

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

// Returns a Report which settings are set
// Used to Print in LogFiles:
QString SettingsHolder::getReport() const {
  QString buff;
  QTextStream out(&buff);
  auto settingsKeys = m_settings.childKeys();
  for (auto setting : settingsKeys) {
    if (SENSITIVE_SETTINGS.contains(setting)) {
      out << setting << " -> <Sensitive>" << Qt::endl;
      continue;
    }
    out << setting << " -> ";
    QVariant value = m_settings.value(setting);
    switch (value.typeId()) {
      case QVariant::List:
      case QVariant::StringList:
        out << '[' << value.toStringList().join(",") << ']' << ' ';
        break;
      default:
        out << value.toString();
    }
    out << Qt::endl;
  }
  return buff;
}

#define SETTING(type, toType, getter, setter, has, key, defvalue, ...)  \
  bool SettingsHolder::has() const { return m_settings.contains(key); } \
  type SettingsHolder::getter() const {                                 \
    if (!has()) {                                                       \
      return defvalue;                                                  \
    }                                                                   \
    return m_settings.value(key).toType();                              \
  }                                                                     \
  void SettingsHolder::setter(const type& value) {                      \
    if (!has() || getter() != value) {                                  \
      m_settings.setValue(key, value);                                  \
      emit getter##Changed(value);                                      \
    }                                                                   \
  }

#include "settingslist.h"
#undef SETTING

QString SettingsHolder::placeholderUserDNS() const {
  return Constants::PLACEHOLDER_USER_DNS;
}

void SettingsHolder::removeEntryServer() {
  m_settings.remove("entryServer/countryCode");
  m_settings.remove("entryServer/city");
}

// Addon specific

void SettingsHolder::clearAddonSettings(const QString& group) {
  logger.debug() << "Clean up the settings for group" << group;

  const QString groupKey(
      QString("%1/%2").arg(Constants::ADDON_SETTINGS_GROUP).arg(group));

  m_settings.beginGroup(groupKey);
  m_settings.remove("");
  m_settings.endGroup();

  emit addonSettingsChanged();
}

// static
QString SettingsHolder::getAddonSettingKey(const AddonSettingQuery& query) {
  return QString("%1/%2/%3/%4")
      .arg(Constants::ADDON_SETTINGS_GROUP)
      .arg(query.m_addonGroup)
      .arg(query.m_addonId)
      .arg(query.m_setting);
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
