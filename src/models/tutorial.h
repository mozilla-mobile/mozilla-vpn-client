/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TUTORIAL_H
#define TUTORIAL_H

#include "itempicker.h"

class TutorialStep;

class Tutorial final : public ItemPicker {
  Q_OBJECT
  Q_PROPERTY(QString titleId MEMBER m_titleId CONSTANT)
  Q_PROPERTY(QString subtitleId MEMBER m_subtitleId CONSTANT)
  Q_PROPERTY(QString completionMessageId MEMBER m_completionMessageId CONSTANT)
  Q_PROPERTY(QString image MEMBER m_image CONSTANT)
  Q_PROPERTY(bool highlighted READ highlighted CONSTANT)

 public:
  ~Tutorial();

  static Tutorial* create(QObject* parent, const QString& fileName);

  void play(const QStringList& allowedItems);
  void stop();

  const QStringList& allowedItems() const { return m_allowedItems; }

  bool highlighted() const { return m_highlighted; }

 private:
  explicit Tutorial(QObject* parent);

  bool itemPicked(const QList<QQuickItem*>& list) override;

  void processNextOp();

  // Return true if there are no operations left.
  bool maybeStop(bool completed = false);

 private:
  QString m_titleId;
  QString m_subtitleId;
  QString m_completionMessageId;
  QString m_image;

  QList<TutorialStep*> m_steps;
  int32_t m_currentStep = -1;
  bool m_highlighted = false;

  QStringList m_allowedItems;
};

#endif  // TUTORIAL_H
