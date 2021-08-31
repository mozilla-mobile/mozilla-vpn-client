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

  WhatsNewModel::getNewFeatures();
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
  roles[NameRole] = "name";
  roles[DisplayNameRole] = "displayName";
  roles[IsSupportedRole] = "isSupported";

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
    case NameRole:
      return QVariant(m_featurelist.at(index.row())->displayName());
    case DisplayNameRole:
      return QVariant(m_featurelist.at(index.row())->displayName());
    case IsSupportedRole:
      return QVariant(m_featurelist.at(index.row())->isSupported());

    default:
      return QVariant();
  }
}

// Get new features
// isNew
// isSupported
// isMajor
void WhatsNewModel::getNewFeatures() {
  m_featurelist = Feature::getAll();

  qDebug() << m_featurelist;

  // QList<Feature *>::iterator iterator = m_featurelist.begin();

  // while (iterator != m_featurelist.end()) {
  //   logger.debug() << "WhatsNewModel - Feature: " << m_featurelist;
  //   ++iterator;
  // }

  for (int i = 0; i < m_featurelist.count(); ++i) {
    // qDebug() << m_featurelist[i];
    // qDebug() << m_featurelist[i]->isNew();
    // qDebug() << m_featurelist[i]->isSupported();
    // qDebug() << m_featurelist[i]->isMajor();

    if (!m_featurelist[i]->isNew() || !m_featurelist[i]->isSupported() ||
        !m_featurelist[i]->isMajor()) {
      m_featurelist.removeAt(i);
    }
  }
}
