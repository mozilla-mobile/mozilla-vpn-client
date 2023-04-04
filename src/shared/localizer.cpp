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
#include <QTranslator>

#include "appconstants.h"
#include "glean/generated/metrics.h"
#include "glean/gleandeprecated.h"
#include "glean/mzglean.h"
#include "languagei18n.h"
#include "leakdetector.h"
#include "logger.h"
#include "settingsholder.h"
#include "telemetry/gleansample.h"

#ifdef MZ_IOS
#  include "platforms/ios/ioscommons.h"
#endif

namespace {
Logger logger("Localizer");
Localizer* s_instance = nullptr;
bool s_forceRTL = false;

// Some languages do not have the right localized/non-localized names in the QT
// framework (and some are missing entirely). This static map is the fallback
// when this happens.
QMap<QString, QString> s_languageMap{
    {"es_AR", "Español, Argentina"},
    {"es_CL", "Español, Chile"},
    {"es_MX", "Español, México"},
};

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
    // By documentation, the language code should be in the format
    // <language>-<script>_<country>. And we don't care about the script.
    QString countryCode;

    QStringList parts = language.split('_');
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
    QString selectedLanguage =
        findLanguageCode(language.first, language.second);
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

  connect(settingsHolder, &SettingsHolder::languageCodeChanged, this,
          &Localizer::settingsChanged);
  settingsChanged();
}

void Localizer::loadLanguagesFromI18n() {
  m_translationCompleteness =
      loadTranslationCompleteness(":/i18n/translations.completeness");

  QDir dir(":/i18n");
  QStringList files = dir.entryList();
  for (const QString& file : files) {
    if (!file.startsWith(AppConstants::LOCALIZER_FILENAME_PREFIX) ||
        !file.endsWith(".qm")) {
      continue;
    }

    QStringList parts = file.split(".");
    Q_ASSERT(parts.length() == 2);

    QString code =
        parts[0].remove(0, strlen(AppConstants::LOCALIZER_FILENAME_PREFIX) +
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
              return LanguageI18N::languageCompare(a.m_code, b.m_code) < 0;
            });
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

  if (!code.isEmpty()) {
    mozilla::glean::sample::non_default_language_used.record(
        mozilla::glean::sample::NonDefaultLanguageUsedExtra{._languageCode =
                                                                code});
    emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
        GleanSample::nonDefaultLanguageUsed, {{"language_code", code}});
  }
  mozilla::glean::settings::using_system_language.set(code.isEmpty());

  m_code = code;

  beginResetModel();
  endResetModel();
}

bool Localizer::loadLanguage(const QString& requestedLocalCode) {
  // Unload the current translators.
  for (QTranslator* translator : m_translators) {
    QCoreApplication::removeTranslator(translator);
    translator->deleteLater();
  }
  m_translators.clear();

  QString localCode = requestedLocalCode;
  if (localCode.isEmpty()) {
    localCode = systemLanguageCode();
  }

  double completeness = m_translationCompleteness.value(localCode, 0);
  if (completeness < 1) {
    logger.debug() << "Let's try to load another language as fallback for code"
                   << localCode;
    maybeLoadLanguageFallback(localCode);
  }

  QLocale locale = QLocale(localCode);
  QLocale::setDefault(locale);

  if (!createTranslator(locale)) {
    logger.error() << "Loading the locale failed - code:" << localCode;
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

  return translator->load(locale, AppConstants::LOCALIZER_FILENAME_PREFIX, "_",
                          ":/i18n");
}

void Localizer::maybeLoadLanguageFallback(const QString& code) {
  if (m_translationFallback.isEmpty()) {
    QFile file(":/i18n/translations_fallback.json");
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

  // First fallback, English where we are 100% sure we have all the
  // translations. If something goes totally wrong, we use English strings.
  if (!createTranslator(QLocale("en"))) {
    logger.warning() << "Loading the fallback locale failed - code: en";
  }

  for (const QString& fallbackCode :
       m_translationFallback.value(code, QStringList())) {
    logger.debug() << "Fallback language:" << fallbackCode;

    if (!createTranslator(QLocale(fallbackCode))) {
      logger.warning() << "Loading the fallback locale failed - code:"
                       << fallbackCode;
    }
  }
}

// static
QString Localizer::nativeLanguageName(const QLocale& locale,
                                      const QString& code) {
#ifndef UNIT_TEST
  if (!Constants::inProduction()) {
    Q_ASSERT_X(LanguageI18N::languageExists(code), "localizer",
               "Languages are out of sync with the translations");
  }
#endif

  // Let's see if we have the translation of this language in this language. We
  // can use it as native language name.
  QString name = LanguageI18N::translateLanguage(code, code);
  if (!name.isEmpty()) {
    return name;
  }

  if (s_languageMap.contains(code)) {
    return s_languageMap[code];
  }

  if (locale.language() == QLocale::C) {
    return "English (US)";
  }

  name = locale.nativeLanguageName();
  if (name.isEmpty()) {
    return locale.languageToString(locale.language());
  }

  // Capitalize the string.
  name.replace(0, 1, locale.toUpper(QString(name[0])));
  return name;
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

QString Localizer::localizedLanguageName(const Language& language) const {
  QString translationCode = SettingsHolder::instance()->languageCode();
  if (translationCode.isEmpty()) {
    translationCode = Localizer::instance()->languageCodeOrSystem();
  }

  QString value =
      LanguageI18N::translateLanguage(translationCode, language.m_code);
  if (!value.isEmpty()) {
    return value;
  }

  // If we don't have 'ab_BC', but we have 'ab'
  if (translationCode.contains('_')) {
    QStringList parts = translationCode.split('_');

    QString value = LanguageI18N::translateLanguage(parts[0], language.m_code);
    if (!value.isEmpty()) {
      return value;
    }
  }

  // Let's ask QT to localize the language.
  QLocale locale(language.m_code);
  if (language.m_code.isEmpty()) {
    locale = QLocale::system();
  }

  return locale.languageToString(locale.language());
}

QVariant Localizer::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  const Language& language = m_languages.at(index.row());

  switch (role) {
    case LocalizedLanguageNameRole:
      return QVariant(localizedLanguageName(language));

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

  // Happy path
  if (locale.currencySymbol(QLocale::CurrencyIsoCode) == currencyIso4217) {
    return locale.toCurrencyString(value);
  }

  QString symbol =
      LanguageI18N::currencySymbolForLanguage(languageCode, currencyIso4217);
  if (!symbol.isEmpty()) {
    return locale.toCurrencyString(value, symbol);
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

// static
void Localizer::forceRTL() { s_forceRTL = true; }
