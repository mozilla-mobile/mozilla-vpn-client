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

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMutableListIterator>

#include <QDebug>

namespace {
Logger logger(LOG_MODEL, "WhatsNewModel");
}

WhatsNewModel::WhatsNewModel() {
  MVPN_COUNT_CTOR(WhatsNewModel);

  WhatsNewModel::setNewFeatures();
}

WhatsNewModel::~WhatsNewModel() { MVPN_COUNT_DTOR(WhatsNewModel); }

int WhatsNewModel::featureCount() {
  logger.debug() << "WhatsNewModel - featureCount: " << m_featurelist.size();
  return m_featurelist.size();
}

QHash<int, QByteArray> WhatsNewModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[RoleId] = "id";
  roles[RoleDisplayName] = "displayName";
  roles[RoleDescription] = "description";
  roles[RoleShortDescription] = "shortDescription";
  roles[RoleImagePath] = "imagePath";
  roles[RoleIconPath] = "iconPath";

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
    case RoleId:
      return QVariant(m_featurelist.at(index.row())->id());
    case RoleDisplayName:
      return QVariant(m_featurelist.at(index.row())->displayName());
    case RoleDescription:
      return QVariant(m_featurelist.at(index.row())->description());
    case RoleShortDescription:
      return QVariant(m_featurelist.at(index.row())->shortDescription());
    case RoleImagePath:
      return QVariant(m_featurelist.at(index.row())->imagePath());
    case RoleIconPath:
      return QVariant(m_featurelist.at(index.row())->iconPath());
    default:
      return QVariant();
  }
}

void WhatsNewModel::setNewFeatures() {
  QList<Feature*> allFeatures = Feature::getAll();
  QList<Feature*> newFeatures;

  for (int i = 0; i < allFeatures.count(); ++i) {
    bool shouldBeInWhatsNew =
        allFeatures[i]->isNew() && allFeatures[i]->isMajor();

    if (shouldBeInWhatsNew) {
      newFeatures.append(allFeatures[i]);
    }
  }

  m_featurelist = newFeatures;
  qDebug() << m_featurelist;
}

bool WhatsNewModel::hasUnseenFeature() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  logger.debug() << "Reading seen features from settings";
  if (settingsHolder->hasSeenFeatures()) {
    const QStringList& seenFeatureList = settingsHolder->seenFeatures();
    logger.debug() << "WhatsNewModel - seenFeatures: " << seenFeatureList;

    for (int i = 0; i < m_featurelist.count(); ++i) {
      bool isSeenFeature = seenFeatureList.contains(m_featurelist[i]->id());

      if (!isSeenFeature) {
        return true;
      }
    }
  }

  emit hasUnseenFeatureChanged();

  return false;
}

void WhatsNewModel::markFeaturesAsSeen() {
  for (int i = 0; i < m_featurelist.count(); ++i) {
    SettingsHolder::instance()->addSeenFeature(m_featurelist[i]->id());
  }

  emit hasUnseenFeatureChanged();
}

void WhatsNewModel::markFeaturesAsUnseen() {
  SettingsHolder::instance()->removeSeenFeatures();

  emit hasUnseenFeatureChanged();
}
