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
  logger.debug() << "create";
  MVPN_COUNT_CTOR(TutorialModel);

  connect(ExternalOpHandler::instance(), &ExternalOpHandler::requestReceived,
          this, &TutorialModel::externalRequestReceived);
}

TutorialModel::~TutorialModel() { MVPN_COUNT_DTOR(TutorialModel); }

bool TutorialModel::createFromJson(const QString& addonId,
                                   const QJsonObject& obj) {
  logger.debug() << "Creation from json";

  Tutorial* tutorial = Tutorial::create(this, obj);
  if (tutorial) {
    beginResetModel();
    m_tutorials.append({addonId, tutorial});
    endResetModel();

    if (tutorial->highlighted()) {
      emit highlightedTutorialChanged();
    }
    return true;
  }

  return false;
}

void TutorialModel::remove(const QString& addonId) {
  for (auto i = m_tutorials.begin(); i != m_tutorials.end(); ++i) {
    if (i->m_addonId == addonId) {
      bool wasHighlighted = i->m_tutorial->highlighted();

      beginResetModel();
      m_tutorials.erase(i);
      endResetModel();

      if (wasHighlighted) {
        emit highlightedTutorialChanged();
      }
      break;
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
      return QVariant::fromValue(m_tutorials.at(index.row()).m_tutorial);

    default:
      return QVariant();
  }
}

void TutorialModel::allowItem(const QString& objectName) {
  m_allowedItems.append(objectName);
}

void TutorialModel::play(Tutorial* tutorial) {
  Q_ASSERT(tutorial);

  stop();

  m_currentTutorial = tutorial;
  emit playingChanged();

  tutorial->play(m_allowedItems);
}

void TutorialModel::stop() {
  if (m_currentTutorial) {
    requireTooltipShown(m_currentTutorial, false);

    m_currentTutorial->stop();
    m_currentTutorial = nullptr;

    emit playingChanged();
  }
}

void TutorialModel::requireTooltipNeeded(Tutorial* tutorial,
                                         const QString& tooltipText,
                                         QObject* targetElement) {
  Q_ASSERT(tutorial);
  Q_ASSERT(tutorial == m_currentTutorial);
  emit tooltipNeeded(tooltipText, targetElement);
}

void TutorialModel::requireTutorialCompleted(
    Tutorial* tutorial, const QString& completionMessageText) {
  Q_ASSERT(tutorial);
  Q_ASSERT(tutorial == m_currentTutorial);
  emit tutorialCompleted(completionMessageText);
}

void TutorialModel::requireTooltipShown(Tutorial* tutorial, bool shown) {
  Q_ASSERT(tutorial);
  Q_ASSERT(tutorial == m_currentTutorial);

  m_tooltipShown = shown;
  emit tooltipShownChanged();
}

Tutorial* TutorialModel::highlightedTutorial() const {
  for (const TutorialData& tutorialData : m_tutorials) {
    if (tutorialData.m_tutorial->highlighted()) {
      return tutorialData.m_tutorial;
    }
  }
  return nullptr;
}

void TutorialModel::externalRequestReceived(ExternalOpHandler::Op op) {
  logger.debug() << "External request received" << op;

  if (!isPlaying()) {
    return;
  }

  if (op != ExternalOpHandler::OpActivate &&
      op != ExternalOpHandler::OpDeactivate &&
      op != ExternalOpHandler::OpNotificationClicked) {
    return;
  }

  stop();
}
