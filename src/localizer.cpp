/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "localizer.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "serveri18n.h"
#include "settingsholder.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLocale>

namespace {
Logger logger(LOG_MAIN, "Localizer");
Localizer* s_instance = nullptr;

struct StaticLanguage {
  QString m_name;
  QString m_localizedName;
};

// Some languages do not have the right localized/non-localized names in the QT
// framework (and some are missing entirely). This static map is the fallback
// when this happens.
QMap<QString, StaticLanguage> s_languageMap{
    {"co", StaticLanguage{"Corsu", ""}},
    {"es_AR", StaticLanguage{"Spanish (Argentina)", "Español, Argentina"}},
    {"es_MX", StaticLanguage{"Spanish (Mexico)", "Español, México"}},
    {"en_GB", StaticLanguage{"English (United Kingdom)", ""}},
    {"en_CA", StaticLanguage{"English (Canada)", ""}},
};

}  // namespace

// static
Localizer* Localizer::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

Localizer::Localizer() {
  MVPN_COUNT_CTOR(Localizer);

  Q_ASSERT(!s_instance);
  s_instance = this;

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  if (settingsHolder->hasLanguageCode()) {
    m_code = settingsHolder->languageCode();
  }

  initialize();
}

Localizer::~Localizer() {
  MVPN_COUNT_DTOR(Localizer);

  Q_ASSERT(s_instance = this);
  s_instance = nullptr;
}

void Localizer::initialize() {
  QString systemCode = QLocale::system().bcp47Name();

  // In previous versions, we did not have the support for the system language.
  // If this is the first time we are here, we need to check if the current
  // language matches with the system one.
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  if (!settingsHolder->hasSystemLanguageCodeMigrated() ||
      !settingsHolder->systemLanguageCodeMigrated()) {
    settingsHolder->setSystemLanguageCodeMigrated(true);

    if (settingsHolder->hasLanguageCode() &&
        settingsHolder->languageCode() == systemCode) {
      settingsHolder->setPreviousLanguageCode(settingsHolder->languageCode());
      settingsHolder->setLanguageCode("");
    }
  }

  // We always need a previous code.
  if (!settingsHolder->hasPreviousLanguageCode() ||
      settingsHolder->previousLanguageCode().isEmpty()) {
    settingsHolder->setPreviousLanguageCode(systemCode);
  }

  loadLanguage(m_code);

  QCoreApplication::installTranslator(&m_translator);
  QDir dir(":/i18n");
  QStringList files = dir.entryList();
  for (const QString& file : files) {
    if (!file.startsWith("mozillavpn_") || !file.endsWith(".qm")) {
      continue;
    }

    QStringList parts = file.split(".");
    Q_ASSERT(parts.length() == 2);

    QString code = parts[0].remove(0, 11);

    Language language{code, languageName(code), localizedLanguageName(code)};
    m_languages.append(language);
  }

  // Sorting languages.
  std::sort(m_languages.begin(), m_languages.end(), languageSort);
}

void Localizer::loadLanguage(const QString& code) {
  logger.log() << "Loading language:" << code;
  if (!loadLanguageInternal(code)) {
    logger.log() << "Loading default language (fallback)";
    loadLanguageInternal("en");
  }

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  if (code.isEmpty() && settingsHolder->hasLanguageCode()) {
    QString previousCode = settingsHolder->languageCode();
    if (!previousCode.isEmpty()) {
      settingsHolder->setPreviousLanguageCode(previousCode);
      emit previousCodeChanged();
    }
  }

  SettingsHolder::instance()->setLanguageCode(code);

  m_code = code;
  emit codeChanged();
}

bool Localizer::loadLanguageInternal(const QString& code) {
  QLocale locale = QLocale(code);
  if (code.isEmpty()) {
    locale = QLocale::system();
  }

  QLocale::setDefault(locale);

  if (!m_translator.load(locale, "mozillavpn", "_", ":/i18n")) {
    logger.log() << "Loading the locale failed."
                 << "code";
    return false;
  }

  return true;
}

// static
QString Localizer::languageName(const QString& code) {
  if (s_languageMap.contains(code)) {
    QString languageName = s_languageMap[code].m_name;
    if (!languageName.isEmpty()) {
      return languageName;
    }
  }

  QLocale locale(code);
  if (code.isEmpty()) {
    locale = QLocale::system();
  }

  if (locale.language() == QLocale::C) {
    return "English (US)";
  }

  QString name = QLocale::languageToString(locale.language());

  // Capitalize the string.
  name.replace(0, 1, locale.toUpper(QString(name[0])));
  return name;
}

// static
QString Localizer::localizedLanguageName(const QString& code) {
  if (s_languageMap.contains(code)) {
    QString languageName = s_languageMap[code].m_localizedName;
    if (!languageName.isEmpty()) {
      return languageName;
    }
  }

  QLocale locale(code);
  if (code.isEmpty()) {
    locale = QLocale::system();
  }

  if (locale.language() == QLocale::C) {
    return "English (US)";
  }

  QString name = locale.nativeLanguageName();
  if (name.isEmpty()) {
    return languageName(code);
  }

  // Capitalize the string.
  name.replace(0, 1, locale.toUpper(QString(name[0])));
  return name;
}

QHash<int, QByteArray> Localizer::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[LanguageRole] = "language";
  roles[LocalizedLanguageRole] = "localizedLanguage";
  roles[CodeRole] = "code";
  return roles;
}

int Localizer::rowCount(const QModelIndex&) const {
  return m_languages.count();
}

QVariant Localizer::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  switch (role) {
    case LanguageRole:
      return QVariant(m_languages.at(index.row()).m_name);

    case LocalizedLanguageRole:
      return QVariant(m_languages.at(index.row()).m_localizedName);

    case CodeRole:
      return QVariant(m_languages.at(index.row()).m_code);

    default:
      return QVariant();
  }
}

QStringList Localizer::languages() const {
  QStringList languages;
  for (const Language& language : m_languages) {
    languages.append(language.m_code);
  }

  return languages;
}

bool Localizer::languageSort(const Localizer::Language& a,
                             const Localizer::Language& b) {
  return a.m_localizedName < b.m_localizedName;
}

QString Localizer::previousCode() const {
  return SettingsHolder::instance()->previousLanguageCode();
}

QString Localizer::translateServerCountry(const QString& countryCode,
                                          const QString& countryName) {
  return ServerI18N::translateCountryName(countryCode, countryName);
}

QString Localizer::translateServerCity(const QString& countryCode,
                                       const QString& cityName) {
  return ServerI18N::translateCityName(countryCode, cityName);
}
