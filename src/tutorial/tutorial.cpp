/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tutorial.h"
#include "addons/addontutorial.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "telemetry/gleansample.h"

#include <QCoreApplication>
#include <QDir>

namespace {
Tutorial* s_instance = nullptr;
Logger logger(LOG_MAIN, "Tutorial");
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
  MVPN_COUNT_CTOR(Tutorial);

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  connect(vpn->controller(), &Controller::readyToServerUnavailable, this,
          &Tutorial::stop);
}

Tutorial::~Tutorial() { MVPN_COUNT_DTOR(Tutorial); }

void Tutorial::allowItem(const QString& objectName) {
  m_allowedItems.append(objectName);
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

  m_currentTutorial->play(m_allowedItems);

  emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
      GleanSample::tutorialStarted, {{"id", m_currentTutorial->id()}});
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
                                    const QString& tooltipText,
                                    QObject* targetElement) {
  Q_ASSERT(tutorial);
  Q_ASSERT(tutorial == m_currentTutorial);
  emit tooltipNeeded(tooltipText, targetElement);

  emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
      GleanSample::tutorialStepViewed,
      {{"tutorial_id", m_currentTutorial->id()}, {"step_id", tooltipText}});
}

void Tutorial::requireTutorialCompleted(AddonTutorial* tutorial,
                                        const QString& completionMessageText) {
  Q_ASSERT(tutorial);
  Q_ASSERT(tutorial == m_currentTutorial);
  emit tutorialCompleted(completionMessageText);

  emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
      GleanSample::tutorialCompleted, {{"id", m_currentTutorial->id()}});
}

void Tutorial::requireTooltipShown(AddonTutorial* tutorial, bool shown) {
  Q_ASSERT(tutorial);
  Q_ASSERT(tutorial == m_currentTutorial);

  m_tooltipShown = shown;
  emit tooltipShownChanged();
}

bool Tutorial::maybeBlockRequest(ExternalOpHandler::Op op) {
  logger.debug() << "External request received" << op;
  Q_ASSERT(isPlaying());

  if (op != ExternalOpHandler::OpActivate &&
      op != ExternalOpHandler::OpDeactivate &&
      op != ExternalOpHandler::OpCloseEvent &&
      op != ExternalOpHandler::OpNotificationClicked) {
    emit interruptRequest(op);
    return true;
  }

  stop();
  return false;
}

void Tutorial::interruptAccepted(ExternalOpHandler::Op op) {
  logger.debug() << "Interrupt by the user";
  stop();
  ExternalOpHandler::instance()->request(op);
}
