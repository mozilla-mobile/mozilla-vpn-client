/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "languagei18n.h"

#include <QCoreApplication>
#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "context/constants.h"
#include "localizer.h"
#include "logging/logger.h"
#include "settings/settingsholder.h"
#include "utilities/leakdetector.h"
#include "utilities/resourceloader.h"

namespace {
Logger logger("LanguageI18N");

QString itemKey(const QString& translationCode, const QString& languageCode) {
  return QString("%1^%2").arg(translationCode, languageCode);
}

}  // namespace

// static
LanguageI18N* LanguageI18N::instance() {
  static LanguageI18N* s_instance = nullptr;
  if (!s_instance) {
    s_instance = new LanguageI18N(qApp);
    s_instance->initialize();
  }

  return s_instance;
}

LanguageI18N::LanguageI18N(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(LanguageI18N);

  connect(ResourceLoader::instance(), &ResourceLoader::cacheFlushNeeded, this,
          [this]() {
            m_languageList.clear();
            m_translations.clear();
            m_currencies.clear();
            initialize();
          });
}

LanguageI18N::~LanguageI18N() { MZ_COUNT_DTOR(LanguageI18N); }

void LanguageI18N::initialize() {
  QFile file(ResourceLoader::instance()->loadFile(":/i18n/languages.json"));
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    logger.error() << "Failed to open the languages.json";
    return;
  }

  QJsonDocument json = QJsonDocument::fromJson(file.readAll());
  if (!json.isArray()) {
    logger.error() << "Invalid format (expected array)";
    return;
  }

  QJsonArray array = json.array();
  for (const QJsonValue& language : array) {
    addLanguage(language);
  }
}

void LanguageI18N::addLanguage(const QJsonValue& value) {
  if (!value.isObject()) {
    return;
  }

  QJsonObject obj = value.toObject();

  QString languageCode = obj["languageCode"].toString();
  if (languageCode.isEmpty()) {
    logger.error() << "Empty languageCode string";
    return;
  }

  QJsonValue translations = obj["languages"];
  if (!translations.isObject()) {
    logger.error() << "Empty translation list";
    return;
  }

  QJsonObject translationObj = translations.toObject();
  for (const QString& translationCode : translationObj.keys()) {
    QString translation(translationObj[translationCode].toString());
    m_translations.insert(itemKey(languageCode, translationCode),
                          translationObj[translationCode].toString());
  }

  QJsonValue currencies = obj["currencies"];
  if (!currencies.isObject()) {
    logger.error() << "Empty currency list";
    return;
  }

  QJsonObject currencyObj = currencies.toObject();
  for (const QString& currencyIso4217 : currencyObj.keys()) {
    m_currencies.insert(itemKey(currencyIso4217, languageCode),
                        currencyObj[currencyIso4217].toString());
  }

  m_languageList.append(languageCode);
}

bool LanguageI18N::languageExists(const QString& languageCode) {
  return m_languageList.contains(languageCode);
}

QString LanguageI18N::translateLanguage(const QString& translationCode,
                                        const QString& languageCode) {
  return m_translations.value(itemKey(translationCode, languageCode));
}

int LanguageI18N::languageCompare(const QString& languageCodeA,
                                  const QString& languageCodeB) {
  qsizetype a = m_languageList.indexOf(languageCodeA);
  qsizetype b = m_languageList.indexOf(languageCodeB);

#ifndef UNIT_TEST
  if (a < 0 || b < 0) {
    // We do not have all the languages in unit-tests
    QByteArray message;
    {
      QTextStream str(&message);
      str << "Unable to find language " << languageCodeA << ":" << a << " or "
          << languageCodeB << ":" << b;
    }
    Q_ASSERT_X(false, "LanguageI18N", message);
  }
#endif

  if (a < b) {
    return -1;
  }

  if (a == b) {
    return 0;
  }

  return 1;
}

QString LanguageI18N::currencySymbolForLanguage(
    const QString& languageCode, const QString& currencyIso4217) {
  return m_currencies.value(itemKey(currencyIso4217, languageCode));
}
