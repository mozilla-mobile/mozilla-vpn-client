/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TUTORIAL_H
#define TUTORIAL_H

#include "itempicker.h"

#include <QTimer>

class TutorialNext;

class Tutorial final : public ItemPicker {
  Q_OBJECT
  Q_PROPERTY(QString titleId MEMBER m_titleId CONSTANT)
  Q_PROPERTY(QString subtitleId MEMBER m_subtitleId CONSTANT)
  Q_PROPERTY(QString completionMessageId MEMBER m_completionMessageId CONSTANT)
  Q_PROPERTY(QString image MEMBER m_image CONSTANT)

 public:
  ~Tutorial();

  static Tutorial* create(QObject* parent, const QString& fileName);

  void play(const QStringList& allowedItems);
  void stop();

 private:
  explicit Tutorial(QObject* parent);

  bool itemPicked(const QStringList& list) override;

  void processNextOp();

  // Return true if there are no operations left.
  bool maybeStop(bool completed = false);

 private:
  int32_t m_currentStep = -1;
  bool m_elementPicked = false;

  QString m_titleId;
  QString m_subtitleId;
  QString m_completionMessageId;
  QString m_image;

  struct Op {
    QString m_element;
    QString m_stringId;
    TutorialNext* m_next;
  };
  QList<Op> m_steps;

  QStringList m_allowedItems;
  QTimer m_timer;
};

#endif  // TUTORIAL_H
