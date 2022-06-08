/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorialstep.h"
#include "guide.h"
#include "inspector/inspectorutils.h"
#include "l18nstrings.h"
#include "leakdetector.h"
#include "logger.h"
#include "tutorial.h"
#include "tutorialstep.h"
#include "tutorialmodel.h"
#include "tutorialstepbefore.h"
#include "tutorialstepnext.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QQuickItem>

namespace {
Logger logger(LOG_MAIN, "TutorialStep");
}

constexpr int TIMEOUT_ITEM_TIMER_MSEC = 300;

// static
TutorialStep* TutorialStep::create(Tutorial* parent, const QString& tutorialId,
                                   const QJsonValue& json) {
  L18nStrings* l18nStrings = L18nStrings::instance();
  Q_ASSERT(l18nStrings);

  QJsonObject obj = json.toObject();

  QString stepId = obj["id"].toString();
  if (stepId.isEmpty()) {
    logger.warning() << "Empty ID step";
    return nullptr;
  }

  stepId = Guide::pascalize(
      QString("tutorial_%1_step_%2").arg(tutorialId).arg(stepId));
  if (!l18nStrings->contains(stepId)) {
    logger.warning() << "No string ID found for the tutorial step" << stepId;
    return nullptr;
  }

  QString element = obj["element"].toString();
  if (element.isEmpty()) {
    logger.warning() << "Empty element for tutorial step" << stepId;
    return nullptr;
  }

  QJsonObject conditions = obj["conditions"].toObject();

  TutorialStepNext* tn = TutorialStepNext::create(parent, obj["next"]);
  if (!tn) {
    logger.warning() << "Unable to parse the 'next' property for tutorial step"
                     << stepId;
    return nullptr;
  }

  // Even if there are no 'before' steps, we always have the visibility check.
  QList<TutorialStepBefore*> tb =
      TutorialStepBefore::create(parent, element, obj["before"]);
  if (tb.isEmpty()) {
    logger.warning()
        << "Unable to parse the 'before' property for tutorial step" << stepId;
    return nullptr;
  }

  return new TutorialStep(parent, element, stepId, conditions, tb, tn);
}

TutorialStep::TutorialStep(Tutorial* parent, const QString& element,
                           const QString& stringId,
                           const QJsonObject& conditions,
                           const QList<TutorialStepBefore*>& before,
                           TutorialStepNext* next)
    : QObject(parent),
      m_parent(parent),
      m_element(element),
      m_stringId(stringId),
      m_conditions(conditions),
      m_before(before),
      m_next(next) {
  MVPN_COUNT_CTOR(TutorialStep);

  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, this, &TutorialStep::startInternal);
}

TutorialStep::~TutorialStep() { MVPN_COUNT_DTOR(TutorialStep); }

void TutorialStep::stop() {
  Q_ASSERT(m_started);
  m_started = false;

  m_timer.stop();

  m_next->disconnect();
  m_next->stop();

  disconnect();
}

void TutorialStep::start() {
  Q_ASSERT(!m_started);
  m_started = true;
  m_currentBeforeStep = 0;

  startInternal();
}

void TutorialStep::startInternal() {
  Q_ASSERT(m_started);

  if (!Guide::evaluateConditions(m_conditions)) {
    logger.info()
        << "Exclude the tutorial step because conditions do not match";
    emit completed();
    return;
  }

  while (m_currentBeforeStep < m_before.length()) {
    if (!m_before[m_currentBeforeStep]->run()) {
      m_timer.start(TIMEOUT_ITEM_TIMER_MSEC);
      return;
    }

    ++m_currentBeforeStep;
  }

  QObject* element = InspectorUtils::findObject(m_element);
  if (!element) {
    m_timer.start(TIMEOUT_ITEM_TIMER_MSEC);
    return;
  }

  QQuickItem* item = qobject_cast<QQuickItem*>(element);
  Q_ASSERT(item);

  // mapRectToScene/Item do not return the correct value. Let's compute the x/y
  // values manually.

  qreal x = item->x();
  qreal y = item->y();
  for (QQuickItem* parent = item->parentItem(); parent;
       parent = parent->parentItem()) {
    x += parent->x();
    y += parent->y();
  }

  TutorialModel* tutorialModel = TutorialModel::instance();
  Q_ASSERT(tutorialModel);

  tutorialModel->requireTooltipShown(m_parent, true);
  tutorialModel->requireTooltipNeeded(
      m_parent, L18nStrings::instance()->value(m_stringId).toString(),
      QRectF(x, y, item->width(), item->height()), m_element);

  connect(m_next, &TutorialStepNext::completed, this, &TutorialStep::completed);
  m_next->start();
}

bool TutorialStep::itemPicked(const QList<QQuickItem*>& list) {
  QObject* element = InspectorUtils::findObject(m_element);
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
