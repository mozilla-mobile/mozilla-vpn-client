/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "serveri18n.h"
#include "logger.h"
#include "settingsholder.h"

#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
Logger logger(LOG_MAIN, "ServerI18N");

bool s_initialized = false;

QHash<QString, QString> s_items;

QString itemKey(const QString& languageCode, const QString& countryCode,
                const QString& city = QString()) {
  return QString("%1^%2^%3").arg(languageCode).arg(countryCode).arg(city);
}

void addCity(const QString& countryCode, const QJsonValue& value) {
  if (!value.isObject()) {
    return;
  }

  QJsonObject obj = value.toObject();

  QString cityName = obj["city"].toString();
  if (cityName.isEmpty()) {
    logger.error() << "Empty city string";
    return;
  }

  QJsonValue languages = obj["languages"];
  if (!languages.isObject()) {
    logger.error() << "Empty language list";
    return;
  }

  QJsonObject languageObj = languages.toObject();
  for (const QString& languageCode : languageObj.keys()) {
    s_items.insert(itemKey(languageCode, countryCode, cityName),
                   languageObj[languageCode].toString());
  }
}

void addCountry(const QJsonValue& value) {
  if (!value.isObject()) {
    return;
  }

  QJsonObject obj = value.toObject();

  QString countryCode = obj["countryCode"].toString();
  if (countryCode.isEmpty()) {
    logger.error() << "Empty countryCode string";
    return;
  }

  QJsonValue languages = obj["languages"];
  if (!languages.isObject()) {
    logger.error() << "Empty language list";
    return;
  }

  QJsonObject languageObj = languages.toObject();
  for (const QString& languageCode : languageObj.keys()) {
    s_items.insert(itemKey(languageCode, countryCode),
                   languageObj[languageCode].toString());
  }

  QJsonValue cities = obj["cities"];
  if (!cities.isArray()) {
    logger.error() << "Empty city list";
    return;
  }

  QJsonArray cityArray = cities.toArray();
  for (const QJsonValue city : cityArray) {
    addCity(countryCode, city);
  }
}

void maybeInitialize() {
  if (s_initialized) {
    return;
  }

  s_initialized = true;

  QFile file(":/i18n/servers.json");
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    logger.error() << "Failed to open the servers.json";
    return;
  }

  QJsonDocument json = QJsonDocument::fromJson(file.readAll());
  if (!json.isArray()) {
    logger.error() << "Invalid format (expected array)";
    return;
  }

  QJsonArray array = json.array();
  for (const QJsonValue country : array) {
    addCountry(country);
  }
}

QString translateItem(const QString& countryCode, const QString& cityName,
                      const QString& fallback) {
  if (!SettingsHolder::instance().hasLanguageCode()) {
    return fallback;
  }

  maybeInitialize();

  QString languageCode = SettingsHolder::instance().languageCode();
  if (languageCode.isEmpty()) {
    languageCode = QLocale::system().bcp47Name();
  }

  QString result = s_items.value(itemKey(languageCode, countryCode, cityName));
  if (!result.isEmpty()) {
    return result;
  }

  // if the language code contains the 'region' part too, we check if we have
  // translations for the whole 'primary language'. Ex: 'de-AT' vs 'de'.
  bool trimmed = false;
  int pos = languageCode.indexOf("-");
  if (pos > 0) {
    languageCode = languageCode.left(pos);
    trimmed = true;
  }

  pos = languageCode.indexOf("_");
  if (pos > 0) {
    languageCode = languageCode.left(pos);
    trimmed = true;
  }

  if (trimmed) {
    result = s_items.value(itemKey(languageCode, countryCode, cityName));
    if (!result.isEmpty()) {
      return result;
    }
  } else {
    // If the language code is not trimmed e.g "es" and we did not have a match
    // so far, lets try itself as region e.g es -> es_ES, de -> de_DE
    QString concat_code = languageCode + "_" + languageCode.toUpper();
    result = s_items.value(itemKey(concat_code, countryCode, cityName));
    if (!result.isEmpty()) {
      return result;
    }
  }
  return fallback;
}

}  // namespace

// static
QString ServerI18N::translateCountryName(const QString& countryCode,
                                         const QString& countryName) {
  return translateItem(countryCode, "", countryName);
}

// static
QString ServerI18N::translateCityName(const QString& countryCode,
                                      const QString& cityName) {
  return translateItem(countryCode, cityName, cityName);
}
