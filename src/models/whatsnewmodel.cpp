/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "whatsnewmodel.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "feature.h"

#include <QList>
#include <QVersionNumber>

namespace {
Logger logger(LOG_MODEL, "WhatsNewModel");
}

WhatsNewModel::WhatsNewModel() {
  MVPN_COUNT_CTOR(WhatsNewModel);

  WhatsNewModel::initialize();
}

WhatsNewModel::~WhatsNewModel() { MVPN_COUNT_DTOR(WhatsNewModel); }

QHash<int, QByteArray> WhatsNewModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[RoleFeature] = "feature";

  return roles;
}

int WhatsNewModel::rowCount(const QModelIndex&) const {
  return m_featurelist.count();
}

QVariant WhatsNewModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  switch (role) {
    case RoleFeature:
      return QVariant::fromValue(m_featurelist.at(index.row()));
    default:
      return QVariant();
  }
}

void WhatsNewModel::initialize() {
  QList<Feature*> allFeatures = Feature::getAll();
  QList<Feature*> newFeatures;

  for (Feature* feature : allFeatures) {
    bool shouldBeInWhatsNew = !feature->displayName().isEmpty() &&
                              !feature->description().isEmpty() &&
                              !feature->imagePath().isEmpty() &&
                              feature->isSupported() && feature->isMajor();

    if (shouldBeInWhatsNew) {
      newFeatures.append(feature);
    }
  }

  m_featurelist = newFeatures;

  // Sort features based on release version, newest to oldest
  std::sort(m_featurelist.begin(), m_featurelist.end(),
            [&](const Feature* a, const Feature* b) -> bool {
              return QVersionNumber::fromString(a->releaseVersion()) >
                     QVersionNumber::fromString(b->releaseVersion());
            });
}

QStringList WhatsNewModel::featureIds() const {
  QStringList featureIds;
  for (const Feature* feature : m_featurelist) {
    featureIds.append(feature->id());
  }

  return featureIds;
}

bool WhatsNewModel::hasUnseenFeature() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  logger.debug() << "Reading seen features from settings";
  const QStringList& seenFeatureList = settingsHolder->seenFeatures();
  if (seenFeatureList.isEmpty()) {
    return false;
  }

  for (Feature* feature : m_featurelist) {
    if (!seenFeatureList.contains(feature->id())) {
      return true;
    }
  }

  return false;
}

void WhatsNewModel::markFeaturesAsSeen() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  logger.debug() << "Add seen features to settings";

  QStringList seenfeatureslist = settingsHolder->seenFeatures();
  for (Feature* feature : m_featurelist) {
    const QString& featureID = feature->id();

    if (!seenfeatureslist.contains(featureID)) {
      seenfeatureslist.append(featureID);
    }
  }

  settingsHolder->setSeenFeatures(seenfeatureslist);

  emit hasUnseenFeatureChanged();
}
