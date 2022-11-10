/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TUTORIALSTEP_H
#define TUTORIALSTEP_H

#include "../addons/addonproperty.h"

#include <QList>
#include <QObject>
#include <QJsonObject>
#include <QTimer>

class AddonConditionWatcher;
class AddonTutorial;
class QJsonValue;
class QQuickItem;
class TutorialStepBefore;
class TutorialStepNext;

class TutorialStep final : public QObject {
  Q_OBJECT

 public:
  static TutorialStep* create(AddonTutorial* parent, const QString& tutorialId,
                              const QJsonValue& json);

  ~TutorialStep();

  void start();
  void stop();

  bool itemPicked(const QList<QQuickItem*>& list);

 signals:
  void completed();

 private:
  TutorialStep(AddonTutorial* parent, const QString& element,
               const QString& stepId, const QString& fallback,
               const QJsonObject& conditions,
               const QList<TutorialStepBefore*>& before,
               TutorialStepNext* next);

  void startInternal();

 private:
  AddonTutorial* m_parent = nullptr;
  AddonConditionWatcher* m_conditionWatcher = nullptr;

  const QString m_stepId;
  const QString m_element;
  AddonProperty m_string;
  const QList<TutorialStepBefore*> m_before;
  TutorialStepNext* m_next = nullptr;

  QTimer m_timer;
  int m_currentBeforeStep = 0;

  bool m_enabled = false;

  enum {
    StateConditionCheck,
    StateBeforeRun,
    StateTooltip,
    StateStopped,
  } m_state = StateStopped;
};

#endif  // TUTORIALSTEP_H
