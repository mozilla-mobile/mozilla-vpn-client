/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TUTORIAL_H
#define TUTORIAL_H

#include "itempicker.h"

#include <QTimer>

class Tutorial final : public ItemPicker {
  Q_OBJECT
  Q_PROPERTY(QString id MEMBER m_id CONSTANT)
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
  bool maybeStop();

 private:
  int32_t m_currentStep = -1;

  QString m_id;
  QString m_image;

  struct Op {
    QString m_element;
    QString m_stringId;
  };
  QList<Op> m_steps;

  QStringList m_allowedItems;
  QTimer m_timer;
};

#endif  // TUTORIAL_H
