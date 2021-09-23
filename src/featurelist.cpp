/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "featurelist.h"
#include "logger.h"
#include "qmlengineholder.h"
#include "settingsholder.h"

#include "features/featureappreview.h"
#include "features/featurecaptiveportal.h"
#include "features/featurecustomdns.h"
#include "features/featureglean.h"
#include "features/featureinappaccountcreate.h"
#include "features/featureinappauth.h"
#include "features/featureinapppurchase.h"
#include "features/featurelocalareaaccess.h"
#include "features/featuremultihop.h"
#include "features/featurenotificationcontrol.h"
#include "features/featuresplittunnel.h"
#include "features/featurestartonboot.h"
#include "features/featureunauthsupport.h"
#include "features/featureunsecurednetworknotification.h"

#include <QJsonDocument>
#include <QJsonObject>
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
  new FeatureGlean();
  new FeatureInAppAccountCreate();
  new FeatureInAppAuth();
  new FeatureInAppPurchase();
  new FeatureLocalAreaAccess();
  new FeatureMultiHop();
  new FeatureNotificationControl();
  new FeatureSplitTunnel();
  new FeatureStartOnBoot();
  new FeatureUnauthSupport();
  new FeatureUnsecuredNetworkNotification();

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
  return Feature::roleNames();
}

int FeatureList::rowCount(const QModelIndex&) const {
  return m_featurelist.size();
}

QVariant FeatureList::data(const QModelIndex& index, int role) const {
  auto feature = m_featurelist.at(index.row());
  if (feature == nullptr) {
    return QVariant();
  }
  return feature->data(role);
};

QObject* FeatureList::get(const QString& feature) {
  const Feature* f = Feature::get(feature);
  auto obj = (QObject*)f;
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
  return obj;
}

void FeatureList::updateFeatureList(const QByteArray& data) {
  QJsonObject json = QJsonDocument::fromJson(data).object();
  QJsonValue unauthSupportEnabled = json["unauthSupportEnabled"];
  if (unauthSupportEnabled.isBool()) {
    logger.debug() << "Setting unauth support enablet to: "
                   << unauthSupportEnabled.toBool();
    FeatureUnauthSupport::instance()->setIsSupported(
        unauthSupportEnabled.toBool());
  } else {
    logger.error() << "Error in parsing unauth support response";
  }
}
