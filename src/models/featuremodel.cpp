/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "featuremodel.h"
#include "feature.h"
#include "logger.h"
#include "qmlengineholder.h"
#include "settingsholder.h"

#ifdef MVPN_ADJUST
#  include "adjust/adjustfiltering.h"
#endif

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QHash>
#include <QProcessEnvironment>
#include <QQmlEngine>

namespace {
FeatureModel* s_instance = nullptr;
Logger logger(LOG_MODEL, "FeatureModel");
}  // namespace

FeatureModel* FeatureModel::instance() {
  if (!s_instance) {
    s_instance = new FeatureModel();
  };
  return s_instance;
}

void FeatureModel::toggleForcedEnable(const QString& feature) {
  logger.debug() << "Flipping on" << feature;

  const Feature* f = Feature::get(feature);
  if (!f) {
    logger.debug() << "Feature" << feature << "does not exist";
    return;
  }

  if (!f->isFlippableOn()) {
    logger.debug() << "Feature" << feature << "cannot be flipped on";
    return;
  }

  if (f->isSupportedIgnoringFlip()) {
    logger.debug() << "This is an internal bug. Why flipping on a pref that is "
                      "already supported?";
    return;
  }

  auto const settings = SettingsHolder::instance();
  QStringList flags = settings->featuresFlippedOn();

  logger.debug() << "Got List - size:" << flags.size();

  if (flags.contains(feature)) {
    logger.debug() << "Contains yes -> remove" << flags.size();
    flags.removeAll(feature);
  } else {
    logger.debug() << "Contains no -> add" << flags.size();
    flags.append(feature);
  }

  settings->setFeaturesFlippedOn(flags);

  logger.debug() << "Feature Flipped! new size:" << flags.size();
  emit dataChanged(createIndex(0, 0), createIndex(Feature::getAll().size(), 0));
}

void FeatureModel::toggleForcedDisable(const QString& feature) {
  logger.debug() << "Flipping off" << feature;

  const Feature* f = Feature::get(feature);
  if (!f) {
    logger.debug() << "Feature" << feature << "does not exist";
    return;
  }

  if (!f->isFlippableOff()) {
    logger.debug() << "Feature" << feature << "cannot be flipped off";
    return;
  }

  if (!f->isSupportedIgnoringFlip()) {
    logger.debug() << "This is an internal bug. Why flipping off a pref that "
                      "is already supported?";
    return;
  }

  auto const settings = SettingsHolder::instance();
  QStringList flags = settings->featuresFlippedOff();

  logger.debug() << "Got List - size:" << flags.size();

  if (flags.contains(feature)) {
    logger.debug() << "Contains yes -> remove" << flags.size();
    flags.removeAll(feature);
  } else {
    logger.debug() << "Contains no -> add" << flags.size();
    flags.append(feature);
  }

  settings->setFeaturesFlippedOff(flags);

  logger.debug() << "Feature Flipped! new size:" << flags.size();
  emit dataChanged(createIndex(0, 0), createIndex(Feature::getAll().size(), 0));
}

QHash<int, QByteArray> FeatureModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[FeatureRole] = "feature";
  return roles;
}

int FeatureModel::rowCount(const QModelIndex&) const {
  return Feature::getAll().size();
}

QVariant FeatureModel::data(const QModelIndex& index, int role) const {
  auto feature = Feature::getAll().at(index.row());
  if (feature == nullptr || role != FeatureRole) {
    return QVariant();
  }

  return QVariant::fromValue(feature);
};

QObject* FeatureModel::get(const QString& feature) {
  const Feature* f = Feature::get(feature);
  auto obj = (QObject*)f;
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
  return obj;
}

void FeatureModel::updateFeatureList(const QByteArray& data) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  QJsonObject json = QJsonDocument::fromJson(data).object();
  if (json.contains("featuresOverwrite")) {
    QJsonValue featuresValue = json["featuresOverwrite"];
    if (!featuresValue.isObject()) {
      logger.error() << "Error in the json format";
      return;
    }

    QStringList featuresFlippedOn;
    QStringList featuresFlippedOff;

    QJsonObject featuresObj = featuresValue.toObject();
    for (const QString& key : featuresObj.keys()) {
      QJsonValue value = featuresObj.value(key);
      if (!value.isBool()) {
        logger.error() << "Error in parsing feature enabling:" << key;
        continue;
      }

      const Feature* feature = Feature::getOrNull(key);
      if (!feature) {
        logger.error() << "No feature named" << key;
        continue;
      }

      if (value.toBool()) {
        if (!feature->isFlippableOn()) {
          logger.error() << "Feature" << key << "cannot be flipped on";
          continue;
        }

        featuresFlippedOn.append(key);
      } else {
        if (!feature->isFlippableOff()) {
          logger.error() << "Feature" << key << "cannot be flipped off";
          continue;
        }

        featuresFlippedOff.append(key);
      }
    }

    settingsHolder->setFeaturesFlippedOn(featuresFlippedOn);
    settingsHolder->setFeaturesFlippedOff(featuresFlippedOff);
  }

#ifdef MVPN_ADJUST
  QJsonValue adjustFieldsValue = json["adjustFields"];
  if (adjustFieldsValue.isUndefined()) {
    logger.debug() << "No adjust fields found in feature list";
    return;
  }

  if (!adjustFieldsValue.isObject()) {
    logger.error()
        << "Error in the json format; adjust fields is not an object";
    return;
  }

  QJsonValue allowParameterValue = adjustFieldsValue["allowParameters"];
  if (!allowParameterValue.isArray()) {
    logger.error()
        << "Error in the json format; allow parameters are not an array";
    return;
  }

  QJsonArray allowParametersArray = allowParameterValue.toArray();
  for (const QJsonValue& param : allowParametersArray) {
    if (!param.isString()) {
      logger.error()
          << "Error in the json format; allowlist parameter is not a string";
      continue;
    }
    AdjustFiltering::instance()->allowField(param.toString());
  }

  QJsonValue denyParameterValue = adjustFieldsValue["denyParameters"];
  if (!denyParameterValue.isObject()) {
    logger.error()
        << "Error in the json format; deny parameters in not an object";
    return;
  }

  QJsonObject denyParameterObject = denyParameterValue.toObject();
  for (const QString& key : denyParameterObject.keys()) {
    QJsonValue value = denyParameterObject.value(key);
    if (!value.isString()) {
      logger.error()
          << "Error in the json format; deny list parameter is not a string";
      continue;
    }

    AdjustFiltering::instance()->denyField(key, value.toString());
  }

  QJsonValue mirrorParameterValue = adjustFieldsValue["mirrorParameters"];
  if (!mirrorParameterValue.isObject()) {
    logger.error()
        << "Error in the json format; mirror parameters are not an object";
    return;
  }

  QJsonObject mirrorParameterObject = mirrorParameterValue.toObject();
  for (const QString& key : mirrorParameterObject.keys()) {
    QJsonValue values = mirrorParameterObject.value(key);
    if (!values.isArray()) {
      logger.error() << "Error in the json format; mirror parameters value is "
                        "not an array";
      continue;
    }

    QJsonArray valuesArray = values.toArray();
    if (valuesArray.size() != 2) {
      logger.error()
          << "Error in the json format; mirror value is not an array";
      continue;
    }

    QJsonValue mirrorParamValue = valuesArray.first();
    if (!mirrorParamValue.isString()) {
      logger.error()
          << "Error in the json format; mirroring field is not a string";
      continue;
    }

    QJsonValue defaultValue = valuesArray.last();
    if (!defaultValue.isString()) {
      logger.error()
          << "Error in the json format; mirror default value is not a string";
      continue;
    }

    AdjustFiltering::instance()->mirrorField(
        key, {mirrorParamValue.toString(), defaultValue.toString()});
  }
#endif
}
