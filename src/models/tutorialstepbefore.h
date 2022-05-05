/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TUTORIALSTEPBEFORE_H
#define TUTORIALSTEPBEFORE_H

#include <QList>
#include <QObject>
#include <QVariant>

class QJsonValue;

class TutorialStepBefore final : public QObject {
  Q_OBJECT

 public:
  static QList<TutorialStepBefore*> create(QObject* parent,
                                           const QString& elementForTooltip,
                                           const QJsonValue& json);

  ~TutorialStepBefore();

  bool run();

 private:
  enum Op {
    PropertySet,
    PropertyGet,
  };

  TutorialStepBefore(QObject* parent, Op op, const QString& element,
                     const QString& property, const QVariant& value);

  bool runPropertyGet();
  bool runPropertySet();

 private:
  const Op m_op;
  const QString m_element;
  const QString m_property;
  const QVariant m_value;
};

#endif  // TUTORIALSTEPBEFORE_H
