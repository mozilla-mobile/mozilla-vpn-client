/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorial.h"

#include <QCoreApplication>
#include <QDir>

#include "addons/addontutorial.h"
#include "app.h"
#include "frontend/navigator.h"
#include "glean/generated/metrics.h"
#include "gleandeprecated.h"
#include "leakdetector.h"
#include "logger.h"
#include "telemetry/gleansample.h"

namespace {
Tutorial* s_instance = nullptr;
Logger logger("Tutorial");
}  // namespace

// static
Tutorial* Tutorial::instance() {
  if (!s_instance) {
    s_instance = new Tutorial(qApp);
  }

  return s_instance;
}

Tutorial::Tutorial(QObject* parent) : QObject(parent) {
  logger.debug() << "create";
  MZ_COUNT_CTOR(Tutorial);

  connect(App::instance(), &App::stateChanged, this, &Tutorial::stop);
}

Tutorial::~Tutorial() { MZ_COUNT_DTOR(Tutorial); }

void Tutorial::allowItem(const QString& objectName) {
  m_allowedItems.append(objectName);
}

void Tutorial::showWarning(Addon* tutorial) {
  Q_ASSERT(tutorial);
  Q_ASSERT(tutorial->type() == "tutorial");

  m_currentTutorial = qobject_cast<AddonTutorial*>(tutorial);
  ExternalOpHandler::instance()->registerBlocker(this);

  if (m_currentTutorial->settingsRollbackNeeded()) {
    emit showWarningNeeded(tutorial);
  }
}

void Tutorial::play(Addon* tutorial) {
  Q_ASSERT(tutorial);
  Q_ASSERT(tutorial->type() == "tutorial");

  stop();

  m_currentTutorial = qobject_cast<AddonTutorial*>(tutorial);

  if (!m_currentTutorial) {
    logger.error() << "Tutorial::play works only with AddonTutorial";
    return;
  }

  ExternalOpHandler::instance()->registerBlocker(this);

  emit playingChanged();

  mozilla::glean::sample::tutorial_started.record(
      mozilla::glean::sample::TutorialStartedExtra{
          ._id = m_currentTutorial->id()});
  emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
      GleanSample::tutorialStarted, {{"id", m_currentTutorial->id()}});

  m_currentTutorial->play(m_allowedItems);
}

void Tutorial::stop() {
  if (m_currentTutorial) {
    requireTooltipShown(m_currentTutorial, false);

    m_currentTutorial->stop();
    m_currentTutorial = nullptr;

    ExternalOpHandler::instance()->unregisterBlocker(this);

    emit playingChanged();
  }
}

void Tutorial::requireTooltipNeeded(AddonTutorial* tutorial,
                                    const QString& stepId, const QString& text,
                                    QObject* targetElement) {
  Q_ASSERT(tutorial);
  Q_ASSERT(tutorial == m_currentTutorial);
  emit tooltipNeeded(text, targetElement);

  mozilla::glean::sample::tutorial_step_viewed.record(
      mozilla::glean::sample::TutorialStepViewedExtra{
          ._stepId = stepId, ._tutorialId = m_currentTutorial->id()});
  emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
      GleanSample::tutorialStepViewed,
      {{"tutorial_id", m_currentTutorial->id()}, {"step_id", stepId}});
}

void Tutorial::requireTutorialCompleted(AddonTutorial* tutorial) {
  Q_ASSERT(tutorial);
  Q_ASSERT(tutorial == m_currentTutorial);
  emit tutorialCompleted(tutorial);

  mozilla::glean::sample::tutorial_completed.record(
      mozilla::glean::sample::TutorialCompletedExtra{._id = tutorial->id()});
  emit GleanDeprecated::instance()->recordGleanEventWithExtraKeys(
      GleanSample::tutorialCompleted, {{"id", tutorial->id()}});
}

void Tutorial::requireTooltipShown(AddonTutorial* tutorial, bool shown) {
  Q_ASSERT(tutorial);
  Q_ASSERT(tutorial == m_currentTutorial);

  m_tooltipShown = shown;
  emit tooltipShownChanged();
}

bool Tutorial::maybeBlockRequest(int op) {
  logger.debug() << "External request received" << op;
  Q_ASSERT(isPlaying());

  if (op == ExternalOpHandler::OpCloseEvent) {
    emit interruptRequest(op);
    return true;
  }

  stop();
  return false;
}

void Tutorial::interruptAccepted(int op) {
  logger.debug() << "Interrupt by the user";
  stop();
  (void)ExternalOpHandler::instance()->request(op);
}
