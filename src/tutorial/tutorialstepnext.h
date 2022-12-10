/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TUTORIALSTEPNEXT_H
#define TUTORIALSTEPNEXT_H

#include <QObject>

class QJsonValue;

class TutorialStepNext final : public QObject {
  Q_OBJECT

 public:
  static TutorialStepNext* create(QObject* parent, const QJsonValue& json);

  /**
   * @brief add a new emitter for tutorial
   *
   * @param group - each emitter must belong to a group (vpn_emitter for
   *                instance)
   * @param emitter - the emitter name
   * @param obj - a QObject that must be kept alive by the calleer
   */
  static void registerEmitter(const QString& group, const QString& emitter,
                              std::function<QObject*()>&& getter);

  ~TutorialStepNext();

  void start();
  void stop();

 private:
  TutorialStepNext(QObject* parent, const QString& qmlEmitter,
                   std::function<QObject*()>&& emitter, const QString& signal);

  void startOrStop(bool start);

 signals:
  void completed();

 private:
  const QString m_qmlEmitter;
  std::function<QObject*()> m_emitterGetter;
  const QString m_signal;
};

#endif  // TUTORIALSTEPNEXT_H
