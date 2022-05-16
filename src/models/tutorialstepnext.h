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

  ~TutorialStepNext();

  void start();
  void stop();

 private:
  // NOTE: If you add new vpn_emitter, please update the documentation.
  enum EmitterType {
    Controller,
    QML,
    SettingsHolder,
  };

  TutorialStepNext(QObject* parent, EmitterType emitterType,
                   const QString& emitter, const QString& signal);

  void startOrStop(bool start);

 signals:
  void completed();

 private:
  const EmitterType m_emitterType;
  const QString m_emitter;
  const QString m_signal;
};

#endif  // TUTORIALSTEPNEXT_H
