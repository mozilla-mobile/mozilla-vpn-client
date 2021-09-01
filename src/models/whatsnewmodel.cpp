/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "whatsnewmodel.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "featurelist.h"
#include "feature.h"

#include <QList>

namespace {
Logger logger(LOG_MODEL, "WhatsNewModel");
}

WhatsNewModel::WhatsNewModel() {
  MVPN_COUNT_CTOR(WhatsNewModel);

  WhatsNewModel::setNewFeatures();
}

WhatsNewModel::~WhatsNewModel() { MVPN_COUNT_DTOR(WhatsNewModel); }

int WhatsNewModel::featureCount() const { return m_featurelist.count(); }

QHash<int, QByteArray> WhatsNewModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[RoleFeature] = "feature";

  return roles;
}

int WhatsNewModel::rowCount(const QModelIndex&) const { return featureCount(); }

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

void WhatsNewModel::setNewFeatures() {
  QList<Feature*> allFeatures = Feature::getAll();
  QList<Feature*> newFeatures;

  for (int i = 0; i < allFeatures.count(); ++i) {
    bool shouldBeInWhatsNew = allFeatures[i]->isNew() &&
                              allFeatures[i]->isMajor() &&
                              allFeatures[i]->isSupported();

    if (shouldBeInWhatsNew) {
      newFeatures.append(allFeatures[i]);
    }
  }

  m_featurelist = newFeatures;
}

bool WhatsNewModel::hasUnseenFeature() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  logger.debug() << "Reading seen features from settings";
  if (!settingsHolder->hasSeenFeatures()) {
    return false;
  }

  const QStringList& seenFeatureList = settingsHolder->seenFeatures();
  for (Feature* feature : m_featurelist) {
    if (!seenFeatureList.contains(feature->id())) {
      return true;
    }
  }

  return false;
}

void WhatsNewModel::markFeaturesAsSeen() {
  for (Feature* feature : m_featurelist) {
    SettingsHolder::instance()->addSeenFeature(feature->id());
  }

  emit hasUnseenFeatureChanged();
}

void WhatsNewModel::markFeaturesAsUnseen() {
  SettingsHolder::instance()->removeSeenFeatures();

  emit hasUnseenFeatureChanged();
}
