/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "localizer.h"
#include "collator.h"
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
    {"es_CL", StaticLanguage{"Spanish (Chile)", "Español, Chile"}},
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
  m_code = settingsHolder->languageCode();

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
  if (!settingsHolder->systemLanguageCodeMigrated()) {
    settingsHolder->setSystemLanguageCodeMigrated(true);

    if (settingsHolder->languageCode() == systemCode) {
      settingsHolder->setPreviousLanguageCode(settingsHolder->languageCode());
      settingsHolder->setLanguageCode("");
    }
  }

  // We always need a previous code.
  if (settingsHolder->previousLanguageCode().isEmpty()) {
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
  Collator collator;
  std::sort(m_languages.begin(), m_languages.end(),
            std::bind(languageSort, std::placeholders::_1,
                      std::placeholders::_2, &collator));
}

void Localizer::loadLanguage(const QString& code) {
  logger.debug() << "Loading language:" << code;
  if (!loadLanguageInternal(code)) {
    logger.debug() << "Loading default language (fallback)";
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
    // On IOS, for some QT issues (to be investigated) we cannot use
    // QLocale::system() directly because it would load the 'en' language
    // instead of the system one. Let's recreate a new QLocale object using the
    // bcp47 code.
    locale = QLocale(QLocale::system().bcp47Name());
  }

  QLocale::setDefault(locale);

  if (!m_translator.load(locale, "mozillavpn", "_", ":/i18n")) {
    logger.error() << "Loading the locale failed - code:" << code;
    return false;
  }

  m_locale = locale;
  emit localeChanged();

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
                             const Localizer::Language& b, Collator* collator) {
  Q_ASSERT(collator);
  return collator->compare(a.m_localizedName, b.m_localizedName) < 0;
}

QString Localizer::previousCode() const {
  return SettingsHolder::instance()->previousLanguageCode();
}

QString Localizer::localizedCityName(const QString& code, const QString& city) {
  return ServerI18N::translateCityName(code, city);
}

// static
void Localizer::macOSInstallerStrings() {
  //% "Mozilla VPN for macOS"
  qtTrId("macosinstaller.title");

  //% "Unable to install"
  qtTrId("macosinstaller.unsupported_version.title");

  //% "Mozilla VPN requires Mac OS X 10.6 or later."
  qtTrId("macosinstaller.unsupported_version.message");

  //% "Previous Installation Detected"
  qtTrId("macosinstaller.previous_build.title");

  //% "A previous installation of Mozilla VPN exists at /Applications/Mozilla "
  //% "VPN.app. This installer will remove the previous installation prior to "
  //% "installing. Please back up any data before proceeding."
  qtTrId("macosinstaller.previous_build.message");

  //% "You will now be guided through the installation steps for the Mozilla "
  //% "VPN. Thank you for choosing your VPN from the trusted pioneer of "
  //% "internet privacy."
  qtTrId("macosinstaller.welcome.message1");

  //% "Click “Continue” to continue the setup."
  qtTrId("macosinstaller.welcome.message2");

  //% "Success!"
  qtTrId("macosinstaller.conclusion.title");

  //% "The Mozilla VPN is successfully installed. You’re ready to start "
  //% "taking control of your online privacy."
  qtTrId("macosinstaller.conclusion.message1_v2");

  //% "Trouble with this installation?"
  qtTrId("macosinstaller.conclusion.message2");

  //% "Get help."
  qtTrId("macosinstaller.conclusion.message3");
}

QString Localizer::localizeCurrency(double value,
                                    const QString& currencyIso4217) {
  QString code = SettingsHolder::instance()->languageCode();
  if (code.isEmpty()) {
    code = QLocale::system().bcp47Name();
  }

  QLocale locale(code);

  if (currencyIso4217.length() != 3) {
    logger.warning() << "Invalid currency iso 4217 value:" << currencyIso4217;
    return locale.toCurrencyString(value, currencyIso4217);
  }

  // Happy path
  if (locale.currencySymbol(QLocale::CurrencyIsoCode) == currencyIso4217) {
    return locale.toCurrencyString(value);
  }

  QString symbol = retrieveCurrencySymbolFallback(currencyIso4217, locale);
  if (symbol.isEmpty()) {
    return locale.toCurrencyString(value, currencyIso4217);
  }

  return locale.toCurrencyString(value, symbol);
}

// static
QString Localizer::retrieveCurrencySymbolFallback(
    const QString& currencyIso4217, const QLocale& currentLocale) {
  // Let's find the locale that matches most of the current locale:
  // - L: language
  // - S: script
  // - C: country
  QList<QLocale> currencyLocalesLSC;
  QList<QLocale> currencyLocalesLS;
  QList<QLocale> currencyLocalesL;
  for (QLocale& l : QLocale::matchingLocales(
           QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry)) {
    if (l.currencySymbol(QLocale::CurrencyIsoCode) != currencyIso4217) continue;

    if (l.language() == currentLocale.language()) {
      if (l.script() == currentLocale.script()) {
        if (l.country() == currentLocale.country()) {
          currencyLocalesLSC.append(l);
          continue;
        }
        currencyLocalesLS.append(l);
        continue;
      }
      currencyLocalesL.append(l);
    }
  }

  if (!currencyLocalesLSC.isEmpty()) {
    logger.warning() << "Fallback LSC" << currencyIso4217
                     << currencyLocalesLSC[0].bcp47Name();
    return currencyLocalesLSC[0].currencySymbol();
  }

  if (!currencyLocalesLS.isEmpty()) {
    logger.warning() << "Fallback LS" << currencyIso4217
                     << currencyLocalesLS[0].bcp47Name();
    return currencyLocalesLS[0].currencySymbol();
  }

  if (!currencyLocalesL.isEmpty()) {
    logger.warning() << "Fallback L" << currencyIso4217
                     << currencyLocalesL[0].bcp47Name();
    return currencyLocalesL[0].currencySymbol();
  }

  logger.warning() << "Fallback not found" << currencyIso4217;
  return currencyIso4217;
}

// static
QString Localizer::majorLanguageCode(const QString& aCode) {
  QString code = aCode;
  if (code.contains("-")) {
    code = code.split("-")[0];
  }

  if (code.contains("_")) {
    code = code.split("_")[0];
  }

  return code;
}
