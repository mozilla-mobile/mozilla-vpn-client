/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addon.h"
#include "addondemo.h"
#include "addonguide.h"
#include "addoni18n.h"
#include "addontutorial.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"
#include "mozillavpn.h"
#include "settingsholder.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
Logger logger(LOG_MAIN, "Addon");

bool evaluateConditionsEnabledFeatures(const QJsonArray& enabledFeatures) {
  for (QJsonValue enabledFeature : enabledFeatures) {
    QString featureName = enabledFeature.toString();

    // If the feature doesn't exist, we crash.
    const Feature* feature = Feature::get(featureName);
    if (!feature) {
      logger.info() << "Feature not found" << featureName;
      return false;
    }

    if (!feature->isSupported()) {
      logger.info() << "Feature not supported" << featureName;
      return false;
    }
  }

  return true;
}

bool evaluateConditionsPlatforms(const QJsonArray& platformArray) {
  QStringList platforms;
  for (QJsonValue platform : platformArray) {
    platforms.append(platform.toString());
  }

  if (!platforms.isEmpty() &&
      !platforms.contains(MozillaVPN::instance()->platform())) {
    logger.info() << "Not supported platform";
    return false;
  }

  return true;
}

bool evaluateConditionsSettingsOp(const QString& op, bool result) {
  if (op == "eq") return result;

  if (op == "neq") return !result;

  logger.warning() << "Invalid settings operator" << op;
  return false;
}

bool evaluateConditionsSettings(const QJsonArray& settings) {
  for (QJsonValue setting : settings) {
    QJsonObject obj = setting.toObject();

    QString op = obj["op"].toString();
    QString key = obj["setting"].toString();
    QVariant valueA = SettingsHolder::instance()->rawSetting(key);
    if (!valueA.isValid()) {
      logger.info() << "Unable to retrieve setting key" << key;
      return false;
    }

    QJsonValue valueB = obj["value"];
    switch (valueB.type()) {
      case QJsonValue::Bool:
        if (!evaluateConditionsSettingsOp(op,
                                          valueA.toBool() == valueB.toBool())) {
          logger.info() << "Setting value doesn't match for key" << key;
          return false;
        }

        break;

      case QJsonValue::Double:
        if (!evaluateConditionsSettingsOp(
                op, valueA.toDouble() == valueB.toDouble())) {
          logger.info() << "Setting value doesn't match for key" << key;
          return false;
        }

        break;
        break;

      case QJsonValue::String:
        if (!evaluateConditionsSettingsOp(
                op, valueA.toString() == valueB.toString())) {
          logger.info() << "Setting value doesn't match for key" << key;
          return false;
        }

        break;

      default:
        logger.warning() << "Unsupported setting value type for key" << key;
        return false;
    }
  }

  return true;
}
}  // namespace

// static
Addon* Addon::create(QObject* parent, const QString& manifestFileName) {
  QFile file(manifestFileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    logger.warning() << "Unable to read the addon manifest of"
                     << manifestFileName;
    return nullptr;
  }

  QJsonDocument json = QJsonDocument::fromJson(file.readAll());
  if (!json.isObject()) {
    logger.warning() << "The manifest must be a JSON document"
                     << manifestFileName;
    return nullptr;
  }

  QJsonObject obj = json.object();

  QString version = obj["api_version"].toString();
  if (version.isEmpty()) {
    logger.warning() << "No API version in the manifest" << manifestFileName;
    return nullptr;
  }

  if (version != "0.1") {
    logger.warning() << "Unsupported API version" << version
                     << manifestFileName;
    return nullptr;
  }

  QJsonObject conditions = obj["conditions"].toObject();
  if (!evaluateConditions(conditions)) {
    logger.info() << "Exclude the addon because conditions do not match"
                  << manifestFileName;
    return nullptr;
  }

  QString id = obj["id"].toString();
  if (id.isEmpty()) {
    logger.warning() << "No id in the manifest" << manifestFileName;
    return nullptr;
  }

  QString name = obj["name"].toString();
  if (name.isEmpty()) {
    logger.warning() << "No name in the manifest" << manifestFileName;
    return nullptr;
  }

  QString type = obj["type"].toString();
  if (type.isEmpty()) {
    logger.warning() << "No type in the manifest" << manifestFileName;
    return nullptr;
  }

  if (!Constants::inProduction() && type == "demo") {
    return AddonDemo::create(parent, manifestFileName, id, name, obj);
  }

  if (type == "i18n") {
    return new AddonI18n(parent, manifestFileName, id, name);
  }

  if (type == "tutorial") {
    return AddonTutorial::create(parent, manifestFileName, id, name, obj);
  }

  if (type == "guide") {
    return AddonGuide::create(parent, manifestFileName, id, name, obj);
  }

  logger.warning() << "Unsupported type" << type << manifestFileName;
  return nullptr;
}

Addon::Addon(QObject* parent, const QString& manifestFileName,
             const QString& id, const QString& name, const QString& type)
    : QObject(parent),
      m_manifestFileName(manifestFileName),
      m_id(id),
      m_name(name),
      m_type(type) {
  MVPN_COUNT_CTOR(Addon);

  QCoreApplication::installTranslator(&m_translator);
  retranslate();
}

Addon::~Addon() {
  MVPN_COUNT_DTOR(Addon);
  QCoreApplication::removeTranslator(&m_translator);
}

void Addon::retranslate() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  QString code = settingsHolder->languageCode();

  QLocale locale = QLocale(code);
  if (code.isEmpty()) {
    locale = QLocale(QLocale::system().bcp47Name());
  }

  if (!m_translator.load(
          locale, "locale", "_",
          QFileInfo(m_manifestFileName).dir().filePath("i18n"))) {
    logger.error() << "Loading the locale failed. - code:" << code;
  }
}

// static
bool Addon::evaluateConditions(const QJsonObject& conditions) {
  if (!evaluateConditionsEnabledFeatures(
          conditions["enabledFeatures"].toArray())) {
    return false;
  }

  if (!evaluateConditionsPlatforms(conditions["platforms"].toArray())) {
    return false;
  }

  if (!evaluateConditionsSettings(conditions["settings"].toArray())) {
    return false;
  }

  return true;
}
