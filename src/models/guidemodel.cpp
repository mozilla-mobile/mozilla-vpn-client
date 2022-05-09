/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "guidemodel.h"
#include "guide.h"
#include "leakdetector.h"
#include "logger.h"

#include <QCoreApplication>
#include <QDir>

namespace {
GuideModel* s_instance = nullptr;
Logger logger(LOG_MAIN, "GuideModel");
}  // namespace

// static
GuideModel* GuideModel::instance() {
  if (!s_instance) {
    s_instance = new GuideModel(qApp);
  }

  return s_instance;
}

GuideModel::GuideModel(QObject* parent) : QAbstractListModel(parent) {
  MVPN_COUNT_CTOR(GuideModel);
  initialize();
}

GuideModel::~GuideModel() { MVPN_COUNT_DTOR(GuideModel); }

void GuideModel::initialize() {
  QDir dir(":/guides");
  QStringList files = dir.entryList();
  files.sort();
  for (const QString& file : files) {
    if (file.endsWith(".json")) {
      Guide* guide = Guide::create(this, dir.filePath(file));
      if (guide) {
        m_guides.append(guide);
      }
    }
  }
}

QHash<int, QByteArray> GuideModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[GuideRole] = "guide";
  return roles;
}

int GuideModel::rowCount(const QModelIndex&) const { return m_guides.count(); }

QVariant GuideModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  switch (role) {
    case GuideRole:
      return QVariant::fromValue(m_guides.at(index.row()));

    default:
      return QVariant();
  }
}
