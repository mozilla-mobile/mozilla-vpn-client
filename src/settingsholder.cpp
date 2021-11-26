/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settingsholder.h"
#include "constants.h"
#include "cryptosettings.h"
#include "featurelist.h"
#include "logger.h"

#include "features/featurecaptiveportal.h"
#include "features/featurelocalareaaccess.h"
#include "features/featuresplittunnel.h"
#include "features/featurestartonboot.h"
#include "features/featureunsecurednetworknotification.h"

#include <QSettings>
#include <QProcessEnvironment>

namespace {

Logger logger(LOG_MAIN, "SettingsHolder");
// Setting Keys That won't show up in a report;
QVector<QString> SENSITIVE_SETTINGS({
    "token", "privateKey",
    "servers",  // Those 2 are not sensitive but
    "devices",  // are more noise then info
});

}  // namespace

// static
SettingsHolder& SettingsHolder::instance() {
  static SettingsHolder instance;
  return instance;
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
  if (!hasInstallationTime()) {
    m_firstExecution = true;
    setInstallationTime(QDateTime::currentDateTime());
  }
}

SettingsHolder::~SettingsHolder() {
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
  }

#include "settingslist.h"
#undef SETTING
}

void SettingsHolder::hardReset() {
  logger.debug() << "Hard reset";
  m_settings.clear();
}

// Returns a Report which settings are set
// Used to Print in LogFiles:
QString SettingsHolder::getReport() {
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
    switch (value.type()) {
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
    m_settings.setValue(key, value);                                    \
    emit getter##Changed(value);                                        \
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

QString SettingsHolder::envOrDefault(const QString& name,
                                     const QString& defaultValue) const {
  QString env;

  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  if (pe.contains(name)) {
    env = pe.value(name);
  }

  if (env.isEmpty()) {
    return defaultValue;
  }

  if (!QUrl(env).isValid()) {
    return defaultValue;
  }

  return env;
}
