/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H

#include <QObject>

class Task;

class TaskScheduler final : public QObject {
  Q_OBJECT

 public:
  static void scheduleTask(Task* task);
  static void deleteTasks();
  static void forceDeleteTasks();

  // This method should never been used! It's used only when we are 100% sure
  // that the current tasks do not conflict with this one.
  static void scheduleTaskNow(Task* task);

 private:
  explicit TaskScheduler(QObject* parent);
  ~TaskScheduler();

  static TaskScheduler* maybeCreate();

  void scheduleTaskInternal(Task* task);
  void deleteTasksInternal(bool forced);

  void maybeRunTask();

  void taskCompleted();

 private:
  Task* m_running_task = nullptr;
  QList<Task*> m_tasks;
};

#endif  // TASKSCHEDULER_H
