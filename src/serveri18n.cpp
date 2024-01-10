/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "serveri18n.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "localizer.h"
#include "logging/logger.h"
#include "resourceloader.h"
#include "settings/settingsholder.h"
#include "utilities/leakdetector.h"

namespace {
Logger logger("ServerI18N");

QString itemKey(const QString& languageCode, const QString& countryCode,
                const QString& city = QString()) {
  return QString("%1^%2^%3").arg(languageCode, countryCode, city);
}
}  // namespace

// static
ServerI18N* ServerI18N::instance() {
  static ServerI18N* s_instance = nullptr;
  if (!s_instance) {
    s_instance = new ServerI18N(qApp);
  }
  return s_instance;
}

ServerI18N::ServerI18N(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(ServerI18N);

  initialize();

  connect(ResourceLoader::instance(), &ResourceLoader::cacheFlushNeeded, this,
          &ServerI18N::initialize);
}

ServerI18N::~ServerI18N() { MZ_COUNT_DTOR(ServerI18N); }

void ServerI18N::initialize() {
  m_items.clear();

  QFile file(ResourceLoader::instance()->loadFile(":/i18n/servers.json"));
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
  for (const QJsonValue& country : array) {
    addCountry(country);
  }
}

QString ServerI18N::translateCountryName(const QString& countryCode,
                                         const QString& countryName) {
  return translateItem(countryCode, "", countryName);
}

QString ServerI18N::translateCityName(const QString& countryCode,
                                      const QString& cityName) {
  return translateItem(countryCode, cityName, cityName);
}

QString ServerI18N::translateItem(const QString& countryCode,
                                  const QString& cityName,
                                  const QString& fallback) {
  QString languageCode = SettingsHolder::instance()->languageCode();
  if (languageCode.isEmpty()) {
    languageCode = Localizer::instance()->languageCodeOrSystem();
  }

  QString result =
      translateItemWithLanguage(languageCode, countryCode, cityName);
  if (!result.isEmpty()) {
    return result;
  }

  if (languageCode != "en") {
    result = translateItemWithLanguage("en", countryCode, cityName);
    if (!result.isEmpty()) {
      return result;
    }
  }

  return fallback;
}

QString ServerI18N::translateItemWithLanguage(const QString& languageCode,
                                              const QString& countryCode,
                                              const QString& cityName) {
  QString languageCodeCopy(languageCode);

  QString result = m_items.value(itemKey(languageCode, countryCode, cityName));
  if (!result.isEmpty()) {
    return result;
  }

  // if the language code contains the 'region' part too, we check if we have
  // translations for the whole 'primary language'. Ex: 'de-AT' vs 'de'.
  bool trimmed = false;
  qsizetype pos = languageCodeCopy.indexOf("-");
  if (pos > 0) {
    languageCodeCopy = languageCodeCopy.left(pos);
    trimmed = true;
  }

  pos = languageCodeCopy.indexOf("_");
  if (pos > 0) {
    languageCodeCopy = languageCodeCopy.left(pos);
    trimmed = true;
  }

  if (trimmed) {
    result = m_items.value(itemKey(languageCodeCopy, countryCode, cityName));
    if (!result.isEmpty()) {
      return result;
    }
  } else {
    // If the language code is not trimmed e.g "es" and we did not have a match
    // so far, lets try itself as region e.g es -> es_ES, de -> de_DE
    QString concat_code = languageCodeCopy + "_" + languageCodeCopy.toUpper();
    result = m_items.value(itemKey(concat_code, countryCode, cityName));
    if (!result.isEmpty()) {
      return result;
    }
  }

  // Let's use 'en' translation, which is still better than what we obtain from
  // Guardian.
  result = m_items.value(itemKey("en", countryCode, cityName));
  if (!result.isEmpty()) {
    return result;
  }

  return QString();
}

void ServerI18N::addCity(const QString& countryCode, const QJsonValue& value) {
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
    m_items.insert(itemKey(languageCode, countryCode, cityName),
                   languageObj[languageCode].toString());
  }
}

void ServerI18N::addCountry(const QJsonValue& value) {
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
    m_items.insert(itemKey(languageCode, countryCode),
                   languageObj[languageCode].toString());
  }

  QJsonValue cities = obj["cities"];
  if (!cities.isArray()) {
    logger.error() << "Empty city list";
    return;
  }

  QJsonArray cityArray = cities.toArray();
  for (const QJsonValue& city : cityArray) {
    addCity(countryCode, city);
  }
}
