/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorialmodel.h"
#include "tutorial.h"
#include "leakdetector.h"
#include "logger.h"

#include <QCoreApplication>
#include <QDir>

namespace {
TutorialModel* s_instance = nullptr;
Logger logger(LOG_MAIN, "TutorialModel");
}  // namespace

// static
TutorialModel* TutorialModel::instance() {
  if (!s_instance) {
    s_instance = new TutorialModel(qApp);
  }

  return s_instance;
}

TutorialModel::TutorialModel(QObject* parent) : QAbstractListModel(parent) {
  MVPN_COUNT_CTOR(TutorialModel);
  initialize();
}

TutorialModel::~TutorialModel() { MVPN_COUNT_DTOR(TutorialModel); }

void TutorialModel::initialize() {
  QDir dir(":/tutorials");
  QStringList files = dir.entryList();
  files.sort();
  for (const QString& file : files) {
    if (file.endsWith(".json")) {
      Tutorial* tutorial = Tutorial::create(this, dir.filePath(file));
      if (tutorial) {
        m_tutorials.append(tutorial);
      }
    }
  }
}

QHash<int, QByteArray> TutorialModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[TutorialRole] = "tutorial";
  return roles;
}

int TutorialModel::rowCount(const QModelIndex&) const {
  return m_tutorials.count();
}

QVariant TutorialModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  switch (role) {
    case TutorialRole:
      return QVariant::fromValue(m_tutorials.at(index.row()));

    default:
      return QVariant();
  }
}
