/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TUTORIALNEXT_H
#define TUTORIALNEXT_H

#include <QObject>

class QJsonValue;

class TutorialNext final : public QObject {
  Q_OBJECT

 public:
  static TutorialNext* create(QObject* parent, const QJsonValue& json);

  ~TutorialNext();

  void start();
  void stop();

 private:
  enum EmitterType {
    SettingsHolder,
    QML,
  };

  TutorialNext(QObject* parent, EmitterType emitterType, const QString& emitter,
               const QString& signal);

  void startOrStop(bool start);

 signals:
  void completed();

 private:
  const EmitterType m_emitterType;
  const QString m_emitter;
  const QString m_signal;
};

#endif  // TUTORIALNEXT_H
