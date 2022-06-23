/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TUTORIALSTEPBEFORE_H
#define TUTORIALSTEPBEFORE_H

#include <QList>
#include <QObject>
#include <QVariant>

class QJsonValue;

class TutorialStepBefore : public QObject {
  Q_OBJECT

 public:
  static QList<TutorialStepBefore*> create(QObject* parent,
                                           const QString& elementForTooltip,
                                           const QJsonValue& json);

  virtual ~TutorialStepBefore();

  virtual bool run() = 0;

 protected:
  explicit TutorialStepBefore(QObject* parent);
};

#endif  // TUTORIALSTEPBEFORE_H
