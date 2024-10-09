/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "localizer.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QRegularExpression>
#include <QTranslator>

#include "constants.h"
#include "glean/generated/metrics.h"
#include "glean/mzglean.h"
#include "i18nlanguagenames.h"
#include "i18nstrings.h"
#include "leakdetector.h"
#include "logger.h"
#include "resourceloader.h"
#include "settingsholder.h"

#ifdef MZ_IOS
#  include "platforms/ios/ioscommons.h"
#endif

namespace {
Logger logger("Localizer");
Localizer* s_instance = nullptr;
bool s_forceRTL = false;

// Fallback map of supported currency symbols.
// The list of supported countries can be found at
// https://mozilla-hub.atlassian.net/wiki/spaces/PXI/pages/173539548/Supported+Markets+and+Currencies.
QMap<QString, QString> s_currencyMap{
    {"USD", "$"},  {"GBP", "£"},   {"NZD", "NZ$"}, {"MYR", "RM"},
    {"SGD", "S$"}, {"CAD", "CA$"}, {"EUR", "€"},   {"CHF", "CHF"},
    {"SEK", "kr"}, {"PLN", "zł"},  {"DKK", "kr"},  {"CZK", "Kč"},
    {"HUF", "Ft"}, {"BGN", "лв"},  {"RON", "lei"}};

QString toUpper(const QLocale& locale, QString input) {
  if (input.isEmpty()) {
    return "";
  }

  return input.replace(0, 1, locale.toUpper(QString(input[0])));
}

QString toPascalCase(const QString& s) {
  QStringList words = s.split("_");

  for (int i = 0; i < words.size(); i++) {
    QString word = words.at(i);
    if (!word.isEmpty()) {
      words[i] = word.at(0).toUpper() + word.mid(1).toLower();
    }
  }

  return words.join("");
}

}  // namespace

// static
QList<QPair<QString, QString>> Localizer::parseBCP47Languages(
    const QStringList& languages) {
  QList<QPair<QString, QString>> codes;

  for (const QString& language : languages) {
    QStringList parts = language.split('-');
    if (parts.length() == 1) {
      codes.append(QPair<QString, QString>{parts[0], QString()});
      continue;
    }

    // Chinese is often reported as `zh-Hant-HK`, and we want the middle
    // section.
    QString script = parts[1];
    if (script == "Hans" || script == "Hant") {
      codes.append(QPair<QString, QString>{parts[0], script});
      continue;
    }

    QString country = parts.last();
    if (country.length() == 2) {
      codes.append(QPair<QString, QString>{parts[0], country});
      continue;
    }

    // Let's ignore all the other language extensions.
    codes.append(QPair<QString, QString>{parts[0], QString()});
  }

  return codes;
}

// static
QList<QPair<QString, QString>> Localizer::parseIOSLanguages(
    const QStringList& languages) {
  QList<QPair<QString, QString>> codes;

  for (const QString& language : languages) {
    // The language code comes in the format <language>-<country> or
    // <language>-<script>-<country>.
    // For an iOS device set to these 6 languages: Chinese Traditional (Macao),
    // Chinese Traditional, Chinese Simplified, English, Mexican Spanish,
    // Spanish
    // ...we get these language codes:
    // [zh-Hant-MO,zh-Hant-US,zh-Hans-US,en-US,es-MX,es-US]
    // iOS shows the 2 part versions for nearly all languages, and shows the
    // 3 part version only when there is a script - like for Chinese variants.
    // Thus, we can pull the second chunk of the locale string in all cases to
    // get the translations flow required by our app. That second chunk gives
    // the country for all languages except Chinese, where it gives the script -
    // and in both these situations, this is what we want (script for Chinese,
    // country for all others).
    QString countryCode;

    QStringList parts = language.split('-');
    if (parts.length() > 1) {
      countryCode = parts[1];
    }

    codes.append(QPair<QString, QString>{parts[0].split('-')[0], countryCode});
  }

  return codes;
}

QString Localizer::systemLanguageCode() const {
#ifdef MZ_IOS
  // iOS (or Qt?) is buggy and QLocale::system().uiLanguages() does not return
  // the list of languages in the preferable order. For some languages (es-GB),
  // en-US is always the preferable one even when it should not be. Let's use
  // custom code here.
  QList<QPair<QString, QString>> uiLanguages =
      parseIOSLanguages(IOSCommons::systemLanguageCodes());
#else
  QList<QPair<QString, QString>> uiLanguages =
      parseBCP47Languages(QLocale::system().uiLanguages());
#endif

  for (const QPair<QString, QString>& language : uiLanguages) {
    QString languagePart = language.first;
    QString localePart = language.second;

    // iOS reports Chinese as "zh-Hans" and "zh-Hant". Android reports
    // them as "zh-CN" and "zh-Hans". However the app uses "zh-CN" and
    // "zh-TW". We need to manually adjust these. More info:
    // https://stackoverflow.com/questions/4892372/language-codes-for-simplified-chinese-and-traditional-chinese
    // (For country-specific variants, it reports it as "zh-Hant-MO", etc. Since
    // the app only looks at the strings before the 1st and 2nd dashes, we
    // ignore the country - which is what we want to do, as we don't have
    // translations for Chinese beyond simplified and traditional.)
    if (languagePart == "zh") {
      if (localePart == "Hans") {
        localePart = "CN";
      }
      if (localePart == "Hant") {
        localePart = "TW";
      }
    }

    QString selectedLanguage = findLanguageCode(languagePart, localePart);
    if (!selectedLanguage.isEmpty()) {
      return selectedLanguage;
    }
  }

  return "en";
}

// static
Localizer* Localizer::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

Localizer::Localizer() {
  MZ_COUNT_CTOR(Localizer);

  Q_ASSERT(!s_instance);
  s_instance = this;

  initialize();
}

Localizer::~Localizer() {
  MZ_COUNT_DTOR(Localizer);

  Q_ASSERT(s_instance = this);
  s_instance = nullptr;
}

void Localizer::initialize() {
  loadLanguagesFromI18n();
  logger.debug() << "Supported languages:" << languages();

  logger.debug() << "System language codes:" << QLocale::system().uiLanguages();

  QString systemCode = systemLanguageCode();
  logger.debug() << "System language code:" << systemCode;

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  // We always need a previous code.
  if (settingsHolder->previousLanguageCode().isEmpty()) {
    settingsHolder->setPreviousLanguageCode(systemCode);
  }

  connect(settingsHolder, &SettingsHolder::languageCodeChanged, this,
          &Localizer::settingsChanged);
  settingsChanged();

  connect(ResourceLoader::instance(), &ResourceLoader::cacheFlushNeeded, this,
          [this]() {
            m_translationFallback.clear();
            m_translationCompleteness.clear();
            m_languages.clear();

            loadLanguagesFromI18n();
          });
}

void Localizer::loadLanguagesFromI18n() {
  beginResetModel();

  m_translationCompleteness =
      loadTranslationCompleteness(":/i18n/translations.completeness");

  QDir dir(ResourceLoader::instance()->loadDir(":/i18n"));
  QStringList files = dir.entryList();
  for (const QString& file : files) {
    if (!file.startsWith(Constants::LOCALIZER_FILENAME_PREFIX) ||
        !file.endsWith(".qm")) {
      continue;
    }

    QStringList parts = file.split(".");
    Q_ASSERT(parts.length() == 2);

    QString code =
        parts[0].remove(0, strlen(Constants::LOCALIZER_FILENAME_PREFIX) +
                               /* the final '_': */ 1);

    if (Constants::inProduction() &&
        m_translationCompleteness.value(code, 0) < 0.7) {
      logger.debug() << "Language excluded:" << code << "completeness:"
                     << m_translationCompleteness.value(code, 0);
      continue;
    }

    QStringList codeParts = code.split("_");

    QLocale locale(code);
    if (code.isEmpty()) {
      locale = QLocale::system();
    }

    Language language{code, codeParts[0],
                      codeParts.length() > 1 ? codeParts[1] : QString(),
                      nativeLanguageName(locale, code),
                      locale.textDirection() == Qt::RightToLeft};
    m_languages.append(language);
  }

  std::sort(m_languages.begin(), m_languages.end(),
            [&](const Language& a, const Language& b) -> bool {
              return a.m_code < b.m_code;
            });

  endResetModel();
}

// static
QMap<QString, double> Localizer::loadTranslationCompleteness(
    const QString& fileName) {
  QFile file(fileName);
  Q_ASSERT(file.exists());

  QMap<QString, double> result;

  if (!file.open(QIODevice::ReadOnly)) {
    logger.warning() << "Unable to open file translations.completeness:"
                     << file.errorString();
    return result;
  }

  QByteArray content = file.readAll();
  for (const QByteArray& line : content.split('\n')) {
    if (!line.contains(':')) continue;

    QList<QByteArray> parts = line.split(':');

    bool ok = false;
    double value = parts[1].toDouble(&ok);
    if (!ok) {
      logger.warning() << "Syntax invalid in translations.completeness, line:"
                       << line;
      continue;
    }

    result.insert(parts[0], value);
  }

  return result;
}

// static
QString Localizer::formatDate(const QDateTime& nowDateTime,
                              const QDateTime& messageDateTime,
                              const QString& yesterday) {
  qint64 diff = messageDateTime.secsTo(nowDateTime);
  if (diff < 0) {
    // The message has a date set in the future...?
    return Localizer::instance()->locale().toString(nowDateTime.time(),
                                                    QLocale::ShortFormat);
  }

  // Today
  if (diff < 86400 && messageDateTime.time() <= nowDateTime.time()) {
    return Localizer::instance()->locale().toString(messageDateTime.time(),
                                                    QLocale::ShortFormat);
  }

  // Yesterday
  if (messageDateTime.date().dayOfYear() ==
          nowDateTime.date().dayOfYear() - 1 ||
      (nowDateTime.date().dayOfYear() == 1 &&
       messageDateTime.date().dayOfYear() ==
           messageDateTime.date().daysInYear())) {
    return yesterday;
  }

  // Before yesterday (but still this week)
  if (messageDateTime.date() >= nowDateTime.date().addDays(-6)) {
    SettingsHolder* settingsHolder = SettingsHolder::instance();
    QString code = settingsHolder->languageCode();
    QLocale locale = QLocale(code);
    return locale.dayName(messageDateTime.date().dayOfWeek());
  }

  // Before this week
  return Localizer::instance()->locale().toString(messageDateTime.date(),
                                                  QLocale::ShortFormat);
}

void Localizer::settingsChanged() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();

  QString code = settingsHolder->languageCode();
  logger.debug() << "Loading language:" << code;

  if (!loadLanguage(code)) {
    logger.debug() << "Loading default language (fallback)";
    loadLanguage("en");
  }

  if (!m_code.isEmpty()) {
    settingsHolder->setPreviousLanguageCode(m_code);
  }

  mozilla::glean::settings::using_system_language.set(code.isEmpty());

  m_code = code;
}

bool Localizer::loadLanguage(const QString& requestedLocalCode) {
  // Unload the current translators.
  for (QTranslator* translator : m_translators) {
    QCoreApplication::removeTranslator(translator);
    translator->deleteLater();
  }
  m_translators.clear();

  QString localeCode = requestedLocalCode;
  if (localeCode.isEmpty()) {
    localeCode = systemLanguageCode();
  }

  maybeLoadLanguageFallback(localeCode);

  QLocale locale = QLocale(localeCode);
  QLocale::setDefault(locale);

  if (!createTranslator(locale)) {
    logger.error() << "Loading the locale failed - code:" << localeCode;
    return false;
  }

  m_locale = locale;
  emit localeChanged();

  return true;
}

bool Localizer::createTranslator(const QLocale& locale) {
  QTranslator* translator = new QTranslator(this);
  m_translators.append(translator);
  QCoreApplication::installTranslator(translator);

  return translator->load(locale, Constants::LOCALIZER_FILENAME_PREFIX, "_",
                          ":/i18n");
}

void Localizer::maybeLoadLanguageFallbackData() {
  if (m_translationFallback.isEmpty()) {
    QFile file(ResourceLoader::instance()->loadFile(
        ":/i18n/translations_fallback.json"));
    Q_ASSERT(file.exists());

    if (!file.open(QIODevice::ReadOnly)) {
      logger.warning() << "Unable to open file translations.fallback:"
                       << file.errorString();
      return;
    }

    QJsonDocument json = QJsonDocument::fromJson(file.readAll());
    QJsonObject obj = json.object();
    for (const QString& key : obj.keys()) {
      QStringList languages;
      for (const QJsonValue& value : obj[key].toArray()) {
        languages.prepend(value.toString());
      }

      m_translationFallback.insert(key, languages);
    }
  }
}

QStringList Localizer::fallbackForLanguage(const QString& code) const {
  return m_translationFallback.value(code, QStringList());
}

void Localizer::maybeLoadLanguageFallback(const QString& code) {
  maybeLoadLanguageFallbackData();

  // First fallback, English where we are 100% sure we have all the
  // translations. If something goes totally wrong, we use English strings.
  if (!createTranslator(QLocale("en"))) {
    logger.warning() << "Loading the fallback locale failed - code: en";
  }

  for (const QString& fallbackCode :
       m_translationFallback.value(code, QStringList())) {
    logger.debug() << "Loading fallback locale:" << fallbackCode;

    if (!createTranslator(QLocale(fallbackCode))) {
      logger.warning() << "Loading the fallback locale failed - code:"
                       << fallbackCode;
    }
  }
}

// static
QString Localizer::nativeLanguageName(const QLocale& locale,
                                      const QString& code) {
  QString localizedLanguageName =
      LanguageStrings::NATIVE_LANGUAGE_NAMES.value(code);
  if (!localizedLanguageName.isEmpty()) {
    return toUpper(locale, localizedLanguageName);
  }

  if (locale.language() == QLocale::C) {
    return "English (US)";
  }

  localizedLanguageName = locale.nativeLanguageName();
  if (localizedLanguageName.isEmpty()) {
    return locale.languageToString(locale.language());
  }

  return toUpper(locale, localizedLanguageName);
}

QHash<int, QByteArray> Localizer::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[LocalizedLanguageNameRole] = "localizedLanguageName";
  roles[NativeLanguageNameRole] = "nativeLanguageName";
  roles[CodeRole] = "code";
  roles[RTLRole] = "isRightToLeft";
  return roles;
}

int Localizer::rowCount(const QModelIndex&) const {
  return static_cast<int>(m_languages.count());
}

QString Localizer::localizedLanguageName(const QString& languageCode) const {
  QString i18nLangId = QString("Languages%1").arg(toPascalCase(languageCode));
  QString value = getCapitalizedStringFromI18n(i18nLangId);

  // Value should never be empty, because the ultimate fallback locale is "en"
  // and that locale has all strings.
  return value;
}

QVariant Localizer::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  const Language& language = m_languages.at(index.row());

  switch (role) {
    case LocalizedLanguageNameRole:
      return QVariant(localizedLanguageName(language.m_code));

    case NativeLanguageNameRole:
      return QVariant(language.m_nativeLanguageName);

    case CodeRole:
      return QVariant(language.m_code);

    case RTLRole:
      return QVariant(language.m_rtl);

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

QString Localizer::languageCodeOrSystem() const {
  QString code = SettingsHolder::instance()->languageCode();
  if (!code.isEmpty()) {
    return code;
  }

  return systemLanguageCode();
}

QString Localizer::localizeCurrency(double value,
                                    const QString& currencyIso4217) {
  QString languageCode = languageCodeOrSystem();
  QLocale locale(languageCode);

  if (currencyIso4217.length() != 3) {
    logger.warning() << "Invalid currency iso 4217 value:" << currencyIso4217;
    return locale.toCurrencyString(value, currencyIso4217);
  }

  if (locale.currencySymbol(QLocale::CurrencyIsoCode) == currencyIso4217) {
    return locale.toCurrencyString(value);
  }

  if (s_currencyMap.contains(currencyIso4217)) {
    return locale.toCurrencyString(value, s_currencyMap[currencyIso4217]);
  }

  return locale.toCurrencyString(value, currencyIso4217);
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

bool Localizer::isRightToLeft() const {
  return s_forceRTL || m_locale.textDirection() == Qt::RightToLeft;
}

QString Localizer::findLanguageCode(const QString& languageCode,
                                    const QString& countryCode) const {
  QString languageCodeWithoutCountry;

  for (const Language& language : m_languages) {
    if (language.m_languageCode != languageCode) {
      continue;
    }

    if (language.m_countryCode == countryCode) {
      return language.m_code;
    }

    if (languageCodeWithoutCountry.isEmpty() ||
        language.m_countryCode.isEmpty() ||
        language.m_languageCode.compare(language.m_countryCode,
                                        Qt::CaseInsensitive) == 0) {
      languageCodeWithoutCountry = language.m_code;
    }
  }

  return languageCodeWithoutCountry;
}

QString Localizer::getTranslatedCountryName(const QString& countryCode,
                                            const QString& countryName) const {
  if (countryCode.isEmpty()) {
    return "";
  }

  // Country name i18n id is: Servers<PascalCaseCountryCode>
  // e.g. ServersDe -> Germany
  QString i18nCountryId = QString("Servers%1").arg(toPascalCase(countryCode));
  auto value = getCapitalizedStringFromI18n(i18nCountryId);

  // The server list is ever changing, so it is plausible that a translation
  // doesn't exist yet for a given server.
  if (value.isEmpty()) {
    return countryName;
  }

  return value;
}

QString Localizer::getTranslatedCityName(const QString& cityName) const {
  if (cityName.isEmpty()) {
    return "";
  }

  // City name i18n id is:
  // Servers<PascalCaseCityNameWithoutStateORSpecialCharacters> e.g.
  // Malmö -> ServersMalm, São Paulo, SP -> ServersSoPaulo, Berlin, BE ->
  // ServersBerlin

  QRegularExpression acceptedChars("[^a-zA-Z ]");
  QString parsedCityName =
      cityName
          .split(u',')[0]              // Remove state suffix
          .replace(acceptedChars, "")  // Remove special characters
          .replace(" ", "_");          // Prepare for toPascalCase

  QString i18nCityId = QString("Servers%1").arg(toPascalCase(parsedCityName));

  auto value = getCapitalizedStringFromI18n(i18nCityId);

  if (cityName.contains("Malm") || cityName.contains("Salt")) {
    logger.debug() << "TEMP:" << cityName << " parsed " << parsedCityName
                   << " i18nCityId " << i18nCityId << " value " << value;
  }

  // The server list is ever changing, so it is plausible that a translation
  // doesn't exist yet for a given server.
  if (value.isEmpty()) {
    return cityName;
  }

  return value;
}

// static
QString Localizer::getTranslationCode() {
  QString translationCode = SettingsHolder::instance()->languageCode();
  if (translationCode.isEmpty()) {
    translationCode = Localizer::instance()->languageCodeOrSystem();
  }

  return translationCode;
}

// static
QString Localizer::getCapitalizedStringFromI18n(const QString& id) {
  QString str = I18nStrings::instance()->t(I18nStrings::getString(id));
  return toUpper(QLocale(getTranslationCode()), str);
}

// static
void Localizer::forceRTL() { s_forceRTL = true; }
