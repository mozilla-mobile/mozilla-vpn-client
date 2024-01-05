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
  worker.start(1 * 1000 * 60 * 60);  // 1hr

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
  worker.start(200);  // 200ms

  // Sleep for 250 milliseconds,
  // give the worker time to schedule the first periodic task.
  QTest::qWait(250);

  auto updatedListOfTasks = TaskScheduler::tasks();

  // We are expecting two tasks, the task scheduled on start + 1.
  QCOMPARE(initialListOfTasks.count() + 2, updatedListOfTasks.count());

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
  worker.start(1 * 1000 * 60 * 60);  // 1hr

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
    worker.start(200);  // 200ms

    // Sleep for 250 milliseconds,
    // give the worker time to schedule the first periodic task.
    QTest::qWait(250);

    auto updatedListOfTasks = TaskScheduler::tasks();

    // We are expecting two tasks, the task scheduled on start + 1.
    QCOMPARE(initialListOfTasks.count() + 2, updatedListOfTasks.count());

    // Check that the tasks are TaskGetFeatureList, just in case.
    // Check that the tasks are TaskGetFeatureList, just in case.
    for (int i = 0; i < 2; ++i) {
      const TaskGetFeatureList* task = dynamic_cast<const TaskGetFeatureList*>(
          updatedListOfTasks.takeLast());
      qDebug() << updatedListOfTasks.count();
      // If it is not a nullptr, the cast was succesfull.
      QVERIFY(task);
    }

    // Update the initial list of tasks before destruction of the worker.
    initialListOfTasks = TaskScheduler::tasks();
  }

  qDebug() << "Heer!";

  // Sleep more 250, if the timer hasn't stopped this will schedule more tasks.
  // But it shouldn't.
  QTest::qWait(250);

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
