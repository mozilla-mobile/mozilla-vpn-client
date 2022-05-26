/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "guide.h"
#include "guideblock.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"
#include "mozillavpn.h"
#include "settingsholder.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScopeGuard>

namespace {
Logger logger(LOG_MAIN, "Guide");

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

Guide::Guide(QObject* parent) : QObject(parent) { MVPN_COUNT_CTOR(Guide); }

Guide::~Guide() { MVPN_COUNT_DTOR(Guide); }

// static
Guide* Guide::create(QObject* parent, const QString& fileName) {
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    logger.warning() << "Unable to read the guide file" << fileName;
    return nullptr;
  }

  QByteArray content = file.readAll();
  QJsonDocument json = QJsonDocument::fromJson(content);
  if (!json.isObject()) {
    logger.warning() << "Invalid JSON file" << fileName;
    return nullptr;
  }

  return create(parent, json.object());
}

// static
Guide* Guide::create(QObject* parent, const QJsonObject& obj) {
  QJsonObject conditions = obj["conditions"].toObject();
  if (!evaluateConditions(conditions)) {
    logger.info() << "Exclude the guide because conditions do not match";
    return nullptr;
  }

  QString guideId = obj["id"].toString();
  if (guideId.isEmpty()) {
    logger.warning() << "Empty ID for guide";
    return nullptr;
  }

  Guide* guide = new Guide(parent);
  auto guard = qScopeGuard([&] { guide->deleteLater(); });

  guide->m_titleId = QString("guide.%1.title").arg(guideId);
  guide->m_subtitleId = QString("guide.%1.subtitle").arg(guideId);

  guide->m_image = obj["image"].toString();
  if (guide->m_image.isEmpty()) {
    logger.warning() << "Empty image for guide";
    return nullptr;
  }

  QJsonValue blocksArray = obj["blocks"];
  if (!blocksArray.isArray()) {
    logger.warning() << "No blocks for guide";
    return nullptr;
  }

  for (QJsonValue blockValue : blocksArray.toArray()) {
    if (!blockValue.isObject()) {
      logger.warning() << "Expected JSON objects as blocks for guide";
      return nullptr;
    }

    QJsonObject blockObj = blockValue.toObject();

    GuideBlock* block = GuideBlock::create(guide, guideId, blockObj);
    if (!block) {
      return nullptr;
    }

    guide->m_blocks.append(block);
  }

  guard.dismiss();
  return guide;
}

// static
bool Guide::evaluateConditions(const QJsonObject& conditions) {
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
