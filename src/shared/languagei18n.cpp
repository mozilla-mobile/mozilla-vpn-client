/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "languagei18n.h"

#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "constants.h"
#include "localizer.h"
#include "logger.h"
#include "settingsholder.h"

namespace {
Logger logger("LanguageI18N");

bool s_initialized = false;

QList<QString> s_languageList;
QHash<QString, QString> s_translations;
QHash<QString, QString> s_currencies;

QString itemKey(const QString& translationCode, const QString& languageCode) {
  return QString("%1^%2").arg(translationCode, languageCode);
}

void addLanguage(const QJsonValue& value) {
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
    s_translations.insert(itemKey(languageCode, translationCode),
                          translationObj[translationCode].toString());
  }

  QJsonValue currencies = obj["currencies"];
  if (!currencies.isObject()) {
    logger.error() << "Empty currency list";
    return;
  }

  QJsonObject currencyObj = currencies.toObject();
  for (const QString& currencyIso4217 : currencyObj.keys()) {
    s_currencies.insert(itemKey(currencyIso4217, languageCode),
                        currencyObj[currencyIso4217].toString());
  }

  s_languageList.append(languageCode);
}

void maybeInitialize() {
  if (s_initialized) {
    return;
  }

  s_initialized = true;

  QFile file(":/i18n/languages.json");
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

}  // namespace

// static
bool LanguageI18N::languageExists(const QString& languageCode) {
  maybeInitialize();
  return s_languageList.contains(languageCode);
}

// static
QString LanguageI18N::translateLanguage(const QString& translationCode,
                                        const QString& languageCode) {
  maybeInitialize();
  return s_translations.value(itemKey(translationCode, languageCode));
}

// static
int LanguageI18N::languageCompare(const QString& languageCodeA,
                                  const QString& languageCodeB) {
  int a = s_languageList.indexOf(languageCodeA);
  int b = s_languageList.indexOf(languageCodeB);

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

// static
QString LanguageI18N::currencySymbolForLanguage(
    const QString& languageCode, const QString& currencyIso4217) {
  maybeInitialize();
  return s_currencies.value(itemKey(currencyIso4217, languageCode));
}
