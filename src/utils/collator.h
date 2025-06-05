/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COLLATOR_H
#define COLLATOR_H

#include <QCollator>
#include <QObject>

class Collator final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Collator)

 public:
  Collator() = default;
  ~Collator() = default;

  int compare(const QString& a, const QString& b);

 private:
  QCollator m_collator;
};

#endif  // COLLATOR_H
