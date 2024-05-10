/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testtaskgetfeaturelistworker.h"

#include <QTest>

#include "feature/taskgetfeaturelist.h"
#include "feature/taskgetfeaturelistworker.h"
#include "settingsholder.h"
#include "task.h"
#include "taskscheduler.h"

void TestTaskGetFeatureListWorker::testTaskIsScheduledOnStart() {
  SettingsHolder settingsHolder;

  // Stop the TaskScheduler so we can inpect the tasks scheduled.
  TaskScheduler::stop();

  auto initialListOfTasks = TaskScheduler::tasks();

  TaskGetFeatureListWorker worker;
  worker.start(std::chrono::milliseconds(1 * 1000 * 60 * 60));  // 1hr

  auto updatedListOfTasks = TaskScheduler::tasks();

  QCOMPARE(initialListOfTasks.count() + 1, updatedListOfTasks.count());

  // Check that the latest task is TaskGetFeatureList, just in case.
  const TaskGetFeatureList* task =
      dynamic_cast<const TaskGetFeatureList*>(updatedListOfTasks.takeLast());
  // If it is not a nullptr, the cast was succesfull.
  QVERIFY(task);

  // Remove the TaskScheduler singleton so that state doesn't leak to other
  // tests.
  TaskScheduler::reset();
}

void TestTaskGetFeatureListWorker::testTaskIsScheduledPeriodically() {
  SettingsHolder settingsHolder;

  // Stop the TaskScheduler so we can inpect the tasks scheduled.
  TaskScheduler::stop();

  auto initialListOfTasks = TaskScheduler::tasks();

  TaskGetFeatureListWorker worker;
  worker.start(std::chrono::milliseconds(200));  // 200ms

  QList<Task*> updatedListOfTasks;
  bool tasksScheduled = false;
  // Each interaion will wait for 100ms. Our timer is for 200ms,
  // we will loop five times just in case. If after 500ms nothing
  // has been scheduled there is definitely something wrong.
  for (int i = 0; i < 5; ++i) {
    // Wait 100ms.
    QTest::qWait(100);
    updatedListOfTasks = TaskScheduler::tasks();
    // Wait for the list to have 2 new tasks.
    // The first one is the init task, the second one is the timer task.
    if (updatedListOfTasks.count() == initialListOfTasks.count() + 2) {
      tasksScheduled = true;
      break;
    }
  }

  // Ensure tasks are scheduled
  QVERIFY(tasksScheduled);

  // Check that the tasks are TaskGetFeatureList, just in case.
  // Check that the tasks are TaskGetFeatureList, just in case.
  for (int i = 0; i < 2; ++i) {
    const TaskGetFeatureList* task =
        dynamic_cast<const TaskGetFeatureList*>(updatedListOfTasks.takeLast());
    // If it is not a nullptr, the cast was succesfull.
    QVERIFY(task);
  }

  // Remove the TaskScheduler singleton so that state doesn't leak to other
  // tests.
  TaskScheduler::reset();
}

void TestTaskGetFeatureListWorker::testTaskIsScheduledOnTokenChange() {
  SettingsHolder settingsHolder;

  // Stop the TaskScheduler so we can inpect the tasks scheduled.
  TaskScheduler::stop();

  auto initialListOfTasks = TaskScheduler::tasks();

  TaskGetFeatureListWorker worker;
  worker.start(std::chrono::milliseconds(1 * 1000 * 60 * 60));  // 1hr

  settingsHolder.setToken("aToken");
  settingsHolder.setToken("anotherToken");
  settingsHolder.removeToken();

  auto updatedListOfTasks = TaskScheduler::tasks();

  // We are expecting four tasks, the task scheduled on start + 3.
  QCOMPARE(initialListOfTasks.count() + 4, updatedListOfTasks.count());

  // Check that the tasks are TaskGetFeatureList, just in case.
  for (int i = 0; i < 4; ++i) {
    const TaskGetFeatureList* task =
        dynamic_cast<const TaskGetFeatureList*>(updatedListOfTasks.takeLast());
    // If it is not a nullptr, the cast was succesfull.
    QVERIFY(task);
  }

  // Remove the TaskScheduler singleton so that state doesn't leak to other
  // tests.
  TaskScheduler::reset();
}

void TestTaskGetFeatureListWorker::testTimerIsStoppedOnDestruction() {
  SettingsHolder settingsHolder;

  // Stop the TaskScheduler so we can inpect the tasks scheduled.
  TaskScheduler::stop();

  auto initialListOfTasks = TaskScheduler::tasks();

  {
    TaskGetFeatureListWorker worker;
    worker.start(std::chrono::milliseconds(200));  // 200ms

    QList<Task*> updatedListOfTasks;
    bool tasksScheduled = false;
    // Each interaion will wait for 100ms. Our timer is for 200ms,
    // we will loop five times just in case. If after 500ms nothing
    // has been scheduled there is definitely something wrong.
    for (int i = 0; i < 5; ++i) {
      // Wait 100ms.
      QTest::qWait(100);
      updatedListOfTasks = TaskScheduler::tasks();
      // Wait for the list to have 2 new tasks.
      // The first one is the init task, the second one is the timer task.
      if (updatedListOfTasks.count() == initialListOfTasks.count() + 2) {
        tasksScheduled = true;
        break;
      }
    }

    // Ensure tasks are scheduled
    QVERIFY(tasksScheduled);

    // Check that the tasks are TaskGetFeatureList, just in case.
    for (int i = 0; i < 2; ++i) {
      const TaskGetFeatureList* task = dynamic_cast<const TaskGetFeatureList*>(
          updatedListOfTasks.takeLast());
      // If it is not a nullptr, the cast was succesfull.
      QVERIFY(task);
    }

    // Update the initial list of tasks before destruction of the worker.
    initialListOfTasks = TaskScheduler::tasks();
  }

  // Sleep more 500ms, if the timer hasn't stopped this will schedule more
  // tasks. But it shouldn't.
  QTest::qWait(500);

  // Also change the token, this should also not schedule any tasks unless
  // the signal connection was done incorrectly.
  settingsHolder.setToken("aToken");

  // The scheduled tasks were consumed above, so this should be back to initial
  // list of tasks size.
  QCOMPARE(initialListOfTasks.count(), TaskScheduler::tasks().count());

  // Remove the TaskScheduler singleton so that state doesn't leak to other
  // tests.
  TaskScheduler::reset();
}

static TestTaskGetFeatureListWorker s_testTaskGetFeatureListWorker;
