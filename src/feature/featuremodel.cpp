/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "featuremodel.h"

#include "feature.h"
#include "logger.h"
#include "qmlengineholder.h"
#include "settingsholder.h"

#ifdef MZ_ADJUST
#  include "adjust/adjustfiltering.h"
#endif

#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QProcessEnvironment>
#include <QQmlEngine>

namespace {
FeatureModel* s_instance = nullptr;
Logger logger("FeatureModel");

void featureToggleOff(const QString& feature, bool add_to_off) {
  auto const settings = SettingsHolder::instance();

  QStringList flags = settings->featuresFlippedOff();
  if (add_to_off) {
    Q_ASSERT(!flags.contains(feature));
    flags.append(feature);
  } else {
    Q_ASSERT(flags.contains(feature));
    flags.removeAll(feature);
  }

  settings->setFeaturesFlippedOff(flags);

  flags = settings->featuresFlippedOn();
  if (flags.contains(feature)) {
    flags.removeAll(feature);
    settings->setFeaturesFlippedOn(flags);
  }
}

void featureToggleOn(const QString& feature, bool add_to_on) {
  auto const settings = SettingsHolder::instance();

  QStringList flags = settings->featuresFlippedOn();
  if (add_to_on) {
    Q_ASSERT(!flags.contains(feature));
    flags.append(feature);
  } else {
    Q_ASSERT(flags.contains(feature));
    flags.removeAll(feature);
  }

  settings->setFeaturesFlippedOn(flags);

  flags = settings->featuresFlippedOff();
  if (flags.contains(feature)) {
    flags.removeAll(feature);
    settings->setFeaturesFlippedOff(flags);
  }
}

}  // namespace

FeatureModel* FeatureModel::instance() {
  if (!s_instance) {
    s_instance = new FeatureModel();
  };
  return s_instance;
}

void FeatureModel::toggle(const QString& feature) {
  logger.debug() << "Toggle feature" << feature;

  const Feature* f = Feature::get(feature);
  if (!f) {
    logger.debug() << "Feature" << feature << "does not exist";
    return;
  }

  // On -> off
  if (f->isSupported()) {
    if (f->isSupportedIgnoringFlip()) {
      // On -> On(+flipped-off)
      if (!f->isFlippableOff()) {
        logger.error() << "This feature should not be toggleable!";
        return;
      }

      featureToggleOff(feature, true);
      emit dataChanged(
          createIndex(0, 0),
          createIndex(static_cast<int>(Feature::getAll().size()), 0));
      return;
    }

    // Off(+flipped-on) -> Off
    featureToggleOn(feature, false);
    emit dataChanged(
        createIndex(0, 0),
        createIndex(static_cast<int>(Feature::getAll().size()), 0));
    return;
  }

  // Off -> on
  if (!f->isSupportedIgnoringFlip()) {
    // Off -> on((+flipped-off)
    if (!f->isFlippableOn()) {
      logger.error() << "This feature should not be toggleable!";
      return;
    }

    featureToggleOn(feature, true);
    emit dataChanged(
        createIndex(0, 0),
        createIndex(static_cast<int>(Feature::getAll().size()), 0));
    return;
  }

  // On(+flipped-off) -> On
  featureToggleOff(feature, false);
  emit dataChanged(createIndex(0, 0),
                   createIndex(static_cast<int>(Feature::getAll().size()), 0));
  return;
}

QHash<int, QByteArray> FeatureModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[FeatureRole] = "feature";
  return roles;
}

int FeatureModel::rowCount(const QModelIndex&) const {
  return static_cast<int>(Feature::getAll().size());
}

QVariant FeatureModel::data(const QModelIndex& index, int role) const {
  auto feature = Feature::getAll().at(index.row());
  if (feature == nullptr || role != FeatureRole) {
    return QVariant();
  }

  return QVariant::fromValue(feature);
};

QObject* FeatureModel::get(const QString& feature) {
  const Feature* f = Feature::getOrNull(feature);
  auto obj = (QObject*)f;
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
  return obj;
}

// static
void FeatureModel::updateFeatures(const QJsonValue& features) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  if (!features.isObject()) {
    logger.error() << "Error in the features json format";
    return;
  }

  QStringList featuresFlippedOn;
  QStringList featuresFlippedOff;

  QJsonObject featuresObj = features.toObject();
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

// static
void FeatureModel::updateExperimentalFeatures(
    const QJsonValue& experimentalFeatures) {
  if (!experimentalFeatures.isObject()) {
    logger.error() << "Error in the json format: experimentalFeatures is"
                      "not an object.";
    return;
  }

  QJsonObject experimentalFeaturesObj = experimentalFeatures.toObject();
  logger.debug() << experimentalFeaturesObj.keys();
  for (const QString& key : experimentalFeaturesObj.keys()) {
    const Feature* experimentalFeature = Feature::getOrNull(key);
    if (!experimentalFeature) {
      logger.warning() << "Got" << key
                       << "but experimental feature doesn't exist. Ignoring.";
      continue;
    }

    QJsonValue experimentalFeatureSettings = experimentalFeaturesObj[key];
    if (!experimentalFeatureSettings.isObject()) {
      logger.error() << "Error in the json format: experimentalFeature" << key
                     << "is not an object.";
      return;
    }

    QJsonObject experimentalFeatureSettingsObj =
        experimentalFeatureSettings.toObject();
    for (const QString& settingKey : experimentalFeatureSettingsObj.keys()) {
      auto value = experimentalFeatureSettingsObj[settingKey].toVariant();
      if (value.isNull()) {
        logger.warning()
            << "Received null value for experimental feature setting"
            << settingKey;
        continue;
      }

      logger.debug() << "Setting experimental feature setting" << settingKey;

      experimentalFeature->settingGroup()->set(settingKey, value);
    }

    logger.debug() << "Toggling experimental feature" << key << "on.";
    featureToggleOn(key, true);
  }
}

void FeatureModel::parseFeatureList(const QByteArray& data) {
  QJsonObject json = QJsonDocument::fromJson(data).object();

  if (json.contains("featuresOverwrite")) {
    QJsonValue features = json["featuresOverwrite"];
    updateFeatures(features);
  }

  if (json.contains("experimentalFeatures")) {
    QJsonValue experimentalFeatures = json["experimentalFeatures"];
    updateExperimentalFeatures(experimentalFeatures);
  }
}
