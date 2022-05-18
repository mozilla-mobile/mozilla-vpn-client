/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TUTORIALSTEP_H
#define TUTORIALSTEP_H

#include <QList>
#include <QObject>
#include <QJsonObject>
#include <QTimer>

class QJsonValue;
class QQuickItem;
class Tutorial;
class TutorialStepBefore;
class TutorialStepNext;

class TutorialStep final : public QObject {
  Q_OBJECT

 public:
  static TutorialStep* create(Tutorial* parent, const QString& tutorialId,
                              const QJsonValue& json);

  ~TutorialStep();

  void start();
  void stop();

  bool itemPicked(const QList<QQuickItem*>& list);

 signals:
  void completed();

 private:
  TutorialStep(Tutorial* parent, const QString& element,
               const QString& stringId, const QJsonObject& conditions,
               const QList<TutorialStepBefore*>& before,
               TutorialStepNext* next);

  void startInternal();

 private:
  Tutorial* m_parent = nullptr;
  const QString m_element;
  const QString m_stringId;
  const QJsonObject m_conditions;
  const QList<TutorialStepBefore*> m_before;
  TutorialStepNext* m_next = nullptr;

  QTimer m_timer;
  bool m_started = false;
  int m_currentBeforeStep = 0;
};

#endif  // TUTORIALSTEP_H
