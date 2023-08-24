/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TUTORIALSTEPNEXT_H
#define TUTORIALSTEPNEXT_H

#include <QObject>

class AddonTutorial;
class QJsonValue;

class TutorialStepNext final : public QObject {
  Q_OBJECT

 public:
  static TutorialStepNext* create(AddonTutorial* parent,
                                  const QJsonValue& json);

  ~TutorialStepNext();

  void start();
  void stop();

  static void registerEmitter(
      const QString& name, bool (*emitterValidator)(const QString& objectName),
      QObject* (*emitter)(const QString& objectName));

 private:
  TutorialStepNext(AddonTutorial* parent, const QString& emitterName,
                   QObject* (*emitterCallback)(const QString&),
                   const QString& signal);

  void startOrStop(bool start);

 signals:
  void completed();

 private:
  AddonTutorial* m_addonTutorial = nullptr;

  const QString m_emitterName;
  QObject* (*m_emitterCallback)(const QString&);
  const QString m_signal;
};

#endif  // TUTORIALSTEPNEXT_H
