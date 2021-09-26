/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASK_H
#define TASK_H

#include <QObject>

class Core;

class Task : public QObject {
  Q_OBJECT

 public:
  explicit Task(const QString& name) : m_name(name) {}
  virtual ~Task() = default;

  const QString& name() const { return m_name; }

  virtual void run(Core* core) = 0;

 signals:
  void completed();

 private:
  QString m_name;
};

#endif  // TASK_H
