/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TUTORIALSTEPBEFORE_H
#define TUTORIALSTEPBEFORE_H

#include <QList>
#include <QObject>
#include <QVariant>

class AddonTutorial;
class QJsonValue;
class QJsonObject;

class TutorialStepBefore : public QObject {
  Q_OBJECT

 public:
  static bool create(AddonTutorial* parent, const QJsonValue& json,
                     QList<TutorialStepBefore*>& list);

  virtual ~TutorialStepBefore();

  virtual bool run() = 0;

  static void registerTutorialStepBefore(
      const QString& name,
      TutorialStepBefore* (*create)(AddonTutorial*, const QJsonObject&));

 protected:
  explicit TutorialStepBefore(AddonTutorial* parent);

 protected:
  AddonTutorial* m_parent = nullptr;
};

#endif  // TUTORIALSTEPBEFORE_H
