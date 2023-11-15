/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorialstep.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QQuickItem>

#include "addons/addontutorial.h"
#include "addons/conditionwatchers/addonconditionwatcher.h"
#include "inspector/inspectorutils.h"
#include "leakdetector.h"
#include "logger.h"
#include "tutorial.h"
#include "tutorialstep.h"
#include "tutorialstepbefore.h"
#include "tutorialstepnext.h"

namespace {
Logger logger("TutorialStep");
}

constexpr int TIMEOUT_ITEM_TIMER_MSEC = 300;

// static
TutorialStep* TutorialStep::create(AddonTutorial* parent,
                                   const QString& tutorialId,
                                   const QJsonValue& json) {
  QJsonObject obj = json.toObject();

  QString stepId = obj["id"].toString();
  if (stepId.isEmpty()) {
    logger.warning() << "Empty ID step";
    return nullptr;
  }

  stepId = QString("tutorial.%1.step.%2").arg(tutorialId, stepId);

  QString query = obj["query"].toString();
  if (query.isEmpty()) {
    logger.warning() << "Empty query for tutorial step" << stepId;
    return nullptr;
  }

  QJsonObject conditions = obj["conditions"].toObject();

  TutorialStepNext* tn = TutorialStepNext::create(parent, obj["next"]);
  if (!tn) {
    logger.warning() << "Unable to parse the 'next' property for tutorial step"
                     << stepId;
    return nullptr;
  }

  QList<TutorialStepBefore*> tb;
  if (!TutorialStepBefore::create(parent, obj["before"], tb)) {
    logger.warning()
        << "Unable to parse the 'before' property for tutorial step" << stepId;
    return nullptr;
  }

  return new TutorialStep(parent, query, stepId, obj["tooltip"].toString(),
                          conditions, tb, tn);
}

TutorialStep::TutorialStep(AddonTutorial* parent, const QString& query,
                           const QString& stepId, const QString& fallback,
                           const QJsonObject& conditions,
                           const QList<TutorialStepBefore*>& before,
                           TutorialStepNext* next)
    : QObject(parent),
      m_parent(parent),
      m_stepId(stepId),
      m_query(query),
      m_before(before),
      m_next(next),
      m_enabled(Addon::evaluateConditions(conditions)) {
  MZ_COUNT_CTOR(TutorialStep);

  m_string.initialize(stepId, fallback);

  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, this, &TutorialStep::startInternal);

  m_conditionWatcher = Addon::maybeCreateConditionWatchers(parent, conditions);
}

TutorialStep::~TutorialStep() { MZ_COUNT_DTOR(TutorialStep); }

void TutorialStep::stop() {
  Q_ASSERT(m_state != StateStopped);
  m_state = StateStopped;

  m_timer.stop();

  m_next->disconnect();
  m_next->stop();

  disconnect();
}

void TutorialStep::start() {
  Q_ASSERT(m_state == StateStopped);
  m_state = StateConditionCheck;

  m_currentBeforeStep = 0;

  startInternal();
}

void TutorialStep::startInternal() {
  Q_ASSERT(m_state != StateStopped);

  switch (m_state) {
    case StateConditionCheck: {
      if (!m_enabled) {
        logger.info()
            << "Exclude the tutorial step because conditions do not match";
        emit completed();
        return;
      }

      if (m_conditionWatcher && !m_conditionWatcher->conditionApplied()) {
        logger.info() << "Exclude the tutorial step because dynamic conditions "
                         "do not match";
        emit completed();
        return;
      }

      m_state = StateBeforeRun;
      [[fallthrough]];
    }

    case StateBeforeRun: {
      while (m_currentBeforeStep < m_before.length()) {
        if (!m_before[m_currentBeforeStep]->run()) {
          m_timer.start(TIMEOUT_ITEM_TIMER_MSEC);
          return;
        }

        ++m_currentBeforeStep;
      }

      m_state = StateTooltip;
      [[fallthrough]];
    }

    case StateTooltip: {
      QObject* element = nullptr;
      if (!element) {
        m_timer.start(TIMEOUT_ITEM_TIMER_MSEC);
        return;
      }

      QQuickItem* item = qobject_cast<QQuickItem*>(element);
      Q_ASSERT(item);

      Tutorial* tutorial = Tutorial::instance();
      Q_ASSERT(tutorial);

      tutorial->requireTooltipShown(m_parent, true);
      tutorial->requireTooltipNeeded(m_parent, m_stepId, m_string.get(),
                                     element);

      connect(m_next, &TutorialStepNext::completed, this,
              &TutorialStep::completed);
      m_next->start();
      break;
    }

    default:
      Q_ASSERT(false);
  }
}

bool TutorialStep::itemPicked(const QList<QQuickItem*>& list) {
  QObject* element = nullptr;
  if (element) {
    QQuickItem* item = qobject_cast<QQuickItem*>(element);
    Q_ASSERT(item);

    if (list.contains(item)) {
      return false;
    }
  }

  for (QQuickItem* item : list) {
    QString objectName = item->objectName();
    if (objectName.isEmpty()) continue;

    if (m_parent->allowedItems().contains(objectName)) {
      return false;
    }
  }

  return true;
}
