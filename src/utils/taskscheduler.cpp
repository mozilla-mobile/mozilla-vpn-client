/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskscheduler.h"

#include <QCoreApplication>
#include <QTimer>

#include "leakdetector.h"
#include "logger.h"
#include "task.h"

namespace {
Logger logger("TaskScheduler");
TaskScheduler* s_taskScheduler = nullptr;
}  // namespace

Q_GLOBAL_STATIC(TaskScheduler, taskScheduler);

// static
void TaskScheduler::scheduleTask(Task* task) {
  Q_ASSERT(task);
  logger.debug() << "Scheduling task:" << task->name();
  taskScheduler->scheduleTaskInternal(task);
}

// static
void TaskScheduler::scheduleTaskNow(Task* task) {
  Q_ASSERT(task);
  logger.debug() << "Scheduling task NOW!:" << task->name();

  task->run();
  connect(task, &Task::completed, task, &QObject::deleteLater);
}

// static
void TaskScheduler::deleteTasks() {
  taskScheduler->deleteTasksInternal(/* forced */ false);
}

// static
void TaskScheduler::forceDeleteTasks() {
  taskScheduler->deleteTasksInternal(/* forced */ true);
}

// static
void TaskScheduler::pause() { taskScheduler->m_paused++; }

// static
void TaskScheduler::resume() {
  taskScheduler->m_paused--;
  taskScheduler->maybeRunTask();
}

// static
QList<Task*>& TaskScheduler::tasks() { return taskScheduler->m_tasks; }

TaskScheduler::TaskScheduler(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(TaskScheduler);
}

TaskScheduler::~TaskScheduler() {
  MZ_COUNT_DTOR(TaskScheduler);
  qDeleteAll(m_tasks);
}

void TaskScheduler::scheduleTaskInternal(Task* task) {
  m_tasks.append(task);
  maybeRunTask();
}

void TaskScheduler::maybeRunTask() {
  logger.debug() << "Tasks: " << m_tasks.size();

  if (m_running_task || m_tasks.empty()) {
    return;
  }

  if (m_paused > 0) {
    return;
  }

  m_running_task = m_tasks.takeFirst();
  Q_ASSERT(m_running_task);

  QObject::connect(m_running_task, &Task::completed, this,
                   &TaskScheduler::taskCompleted);

  m_running_task->run();
}

void TaskScheduler::taskCompleted() {
  Q_ASSERT(m_running_task);

  logger.debug() << "Task completed:" << m_running_task->name();
  m_running_task->deleteLater();
  m_running_task->disconnect();
  m_running_task = nullptr;

  maybeRunTask();
}

void TaskScheduler::deleteTasksInternal(bool forced) {
  QMutableListIterator<Task*> i(m_tasks);
  while (i.hasNext()) {
    Task* task = i.next();

    if (forced) {
      task->deleteLater();
      i.remove();
      continue;
    }

    switch (task->deletePolicy()) {
      case Task::Deletable:
        task->deleteLater();
        i.remove();
        break;

      case Task::NonDeletable:
        // Nothing to do.
        break;

      case Task::Reschedulable:
        QTimer::singleShot(0, this,
                           [this, task]() { scheduleTaskInternal(task); });
        i.remove();
        break;
    }
  }

  if (m_running_task) {
    if (forced || m_running_task->deletePolicy() == Task::Deletable) {
      m_running_task->cancel();
      m_running_task->deleteLater();
      m_running_task->disconnect();
      m_running_task = nullptr;
    }
  }

  maybeRunTask();
}
