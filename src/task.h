/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASK_H
#define TASK_H

#include <QObject>

class Task : public QObject {
  Q_OBJECT

 public:
  enum DeletePolicy {
    // if TaskScheduler::deleteTasks() is called, this task can be deleted.
    // This is the default policy.
    Deletable,

    // When we are about to delete tasks, this must be kept in the queue.
    NonDeletable,

    // This task can be schedule at any time. Instead of deleting it, it is
    // rescheduled after a few millisecs.
    Reschedulable,
  };

  explicit Task(const QString& name) : m_name(name) {}
  virtual ~Task() = default;

  const QString& name() const { return m_name; }

  virtual void run() = 0;
  virtual void cancel() { m_cancelled = true; }

  // Overwrite this method if the task should not be deleted before being
  // executed.
  virtual DeletePolicy deletePolicy() const { return Deletable; }

 signals:
  void completed();

 protected:
  bool m_cancelled = false;

 private:
  QString m_name;
};

#endif  // TASK_H
