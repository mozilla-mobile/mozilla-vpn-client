/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "featurelist.h"
#include "logger.h"
#include "qmlengineholder.h"
#include "settingsholder.h"

#ifdef MVPN_ADJUST
#  include "adjust/adjustfiltering.h"
#endif

#include "features/featureappreview.h"
#include "features/featurecaptiveportal.h"
#include "features/featurecustomdns.h"
#include "features/featureinappaccountcreate.h"
#include "features/featureinappauth.h"
#include "features/featureinapppurchase.h"
#include "features/featurelocalareaaccess.h"
#include "features/featuremultiaccountcontainers.h"
#include "features/featuremultihop.h"
#include "features/featurenotificationcontrol.h"
#include "features/featuresplittunnel.h"
#include "features/featuresharelogs.h"
#include "features/featureuniqueid.h"
#include "features/featurestartonboot.h"
#include "features/featureunsecurednetworknotification.h"
#include "features/featureserverunavailablenotification.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QHash>
#include <QProcessEnvironment>

namespace {
FeatureList* s_instance = nullptr;
Logger logger(LOG_MODEL, "FeatureList");
}  // namespace

FeatureList* FeatureList::instance() {
  if (!s_instance) {
    s_instance = new FeatureList();
  };
  return s_instance;
}

void FeatureList::initialize() {
  new FeatureAppReview();
  new FeatureCaptivePortal();
  new FeatureCustomDNS();
  new FeatureInAppAccountCreate();
  new FeatureInAppAuth();
  new FeatureInAppPurchase();
  new FeatureLocalAreaAccess();
  new FeatureMultiAccountContainers();
  new FeatureMultiHop();
  new FeatureNotificationControl();
  new FeatureShareLogs();
  new FeatureSplitTunnel();
  new FeatureStartOnBoot();
  new FeatureUniqueID();
  new FeatureUnsecuredNetworkNotification();
  new FeatureServerUnavailableNotification();

  m_featurelist = Feature::getAll();
}

void FeatureList::devModeFlipFeatureFlag(const QString& feature) {
  logger.debug() << "Flipping " << feature;

  auto const settings = SettingsHolder::instance();
  QStringList flags = settings->devModeFeatureFlags();

  logger.debug() << "Got List - size:" << flags.size();

  if (flags.contains(feature)) {
    logger.debug() << "Contains yes -> remove" << flags.size();
    flags.removeAll(feature);
  } else {
    logger.debug() << "Contains no -> add" << flags.size();
    flags.append(feature);
  }

  settings->setDevModeFeatureFlags(flags);

  logger.debug() << "Feature Flipped! new size:" << flags.size();
  emit dataChanged(createIndex(0, 0), createIndex(m_featurelist.size(), 0));
}

QHash<int, QByteArray> FeatureList::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[FeatureRole] = "feature";
  return roles;
}

int FeatureList::rowCount(const QModelIndex&) const {
  return m_featurelist.size();
}

QVariant FeatureList::data(const QModelIndex& index, int role) const {
  auto feature = m_featurelist.at(index.row());
  if (feature == nullptr || role != FeatureRole) {
    return QVariant();
  }

  return QVariant::fromValue(feature);
};

QObject* FeatureList::get(const QString& feature) {
  const Feature* f = Feature::get(feature);
  auto obj = (QObject*)f;
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
  return obj;
}

void FeatureList::updateFeatureList(const QByteArray& data) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  bool changed = false;
  QStringList devModeFeatureFlags = settingsHolder->devModeFeatureFlags();

  QJsonObject json = QJsonDocument::fromJson(data).object();
  QJsonValue featuresValue = json["features"];
  if (!featuresValue.isObject()) {
    logger.error() << "Error in the json format";
    return;
  }

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

    if (value.toBool() == false) {
      if (devModeFeatureFlags.contains(key)) {
        devModeFeatureFlags.removeAll(key);
        changed = true;
      }
    } else if (!devModeFeatureFlags.contains(key)) {
      devModeFeatureFlags.append(key);
      changed = true;
    }
  }

  if (changed) {
    settingsHolder->setDevModeFeatureFlags(devModeFeatureFlags);
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
