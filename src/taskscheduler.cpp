/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskscheduler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "task.h"

namespace {
Logger logger(LOG_MAIN, "TaskScheduler");
}  // namespace

// static
void TaskScheduler::scheduleTask(Task* task) {
  Q_ASSERT(task);
  logger.debug() << "Scheduling task: " << task->name();
  maybeCreate()->scheduleTaskInternal(task);
}

// static
void TaskScheduler::deleteTasks() { maybeCreate()->deleteTasksInternal(); }

// static
TaskScheduler* TaskScheduler::maybeCreate() {
  static TaskScheduler* s_taskScheduler = nullptr;
  if (!s_taskScheduler) {
    s_taskScheduler = new TaskScheduler(&MozillaVPN::instance());
  }
  return s_taskScheduler;
}

TaskScheduler::TaskScheduler(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(TaskScheduler);
}

TaskScheduler::~TaskScheduler() { MVPN_COUNT_DTOR(TaskScheduler); }

void TaskScheduler::scheduleTaskInternal(Task* task) {
  m_tasks.append(task);
  maybeRunTask();
}

void TaskScheduler::maybeRunTask() {
  logger.debug() << "Tasks: " << m_tasks.size();

  if (m_running_task || m_tasks.empty()) {
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

void TaskScheduler::deleteTasksInternal() {
  QMutableListIterator<Task*> i(m_tasks);
  while (i.hasNext()) {
    Task* task = i.next();
    if (task->deletable()) {
      task->deleteLater();
      i.remove();
    }
  }

  if (m_running_task && m_running_task->deletable()) {
    m_running_task->cancel();
    m_running_task->deleteLater();
    m_running_task->disconnect();
    m_running_task = nullptr;
  }
}
