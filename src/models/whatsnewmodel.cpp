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
  // Start: Get seen features from settings
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  logger.debug() << "Reading seen features from settings";
  if (settingsHolder->hasSeenFeatures()) {
    const QStringList& seenFeatureList = settingsHolder->seenFeatures();
    logger.debug() << "WhatsNewModel - seenFeatures: " << seenFeatureList;
  }
  // End: Get seen features from settings

  logger.debug() << "WhatsNewModel - featureCount: " << m_featurelist.size();
  return m_featurelist.size();
}

QHash<int, QByteArray> WhatsNewModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[RoleId] = "id";
  roles[RoleName] = "name";
  roles[RoleIsSupported] = "isSupported";

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
    case RoleName:
      return QVariant(m_featurelist.at(index.row())->displayName());
    case RoleIsSupported:
      return QVariant(m_featurelist.at(index.row())->isSupported());

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
