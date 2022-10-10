/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testtasks.h"
#include "../../src/mozillavpn.h"
#include "../../src/tasks/account/taskaccount.h"
#include "../../src/tasks/adddevice/taskadddevice.h"
#include "../../src/tasks/function/taskfunction.h"
#include "../../src/tasks/group/taskgroup.h"
#include "../../src/tasks/servers/taskservers.h"
#include "../../src/taskscheduler.h"

void TestTasks::account() {
  // Failure
  {
    TestHelper::networkConfig.append(TestHelper::NetworkConfig(
        TestHelper::NetworkConfig::Failure, QByteArray()));

    TaskAccount* task = new TaskAccount(ErrorHandler::PropagateError);

    QEventLoop loop;
    connect(task, &Task::completed, [&]() { loop.exit(); });

    TaskScheduler::scheduleTask(task);
    loop.exec();
  }

  // Success
  {
    TestHelper::networkConfig.append(TestHelper::NetworkConfig(
        TestHelper::NetworkConfig::Success, QByteArray()));

    TaskAccount* task = new TaskAccount(ErrorHandler::NoErrorPropagation);

    QEventLoop loop;
    connect(task, &Task::completed, [&]() { loop.exit(); });

    TaskScheduler::scheduleTask(task);
    loop.exec();
  }
}

void TestTasks::servers() {
  // Failure
  {
    TestHelper::networkConfig.append(TestHelper::NetworkConfig(
        TestHelper::NetworkConfig::Failure, QByteArray()));

    TaskServers* task = new TaskServers(ErrorHandler::NoErrorPropagation);

    QEventLoop loop;
    connect(task, &Task::completed, [&]() { loop.exit(); });

    TaskScheduler::scheduleTask(task);
    loop.exec();
  }

  // Success
  {
    TestHelper::networkConfig.append(TestHelper::NetworkConfig(
        TestHelper::NetworkConfig::Success, QByteArray()));

    TaskServers* task = new TaskServers(ErrorHandler::PropagateError);

    QEventLoop loop;
    connect(task, &Task::completed, [&]() { loop.exit(); });

    TaskScheduler::scheduleTask(task);
    loop.exec();
  }
}

void TestTasks::addDevice_success() {
  TestHelper::networkConfig.append(TestHelper::NetworkConfig(
      TestHelper::NetworkConfig::Success, QByteArray()));

  TaskAddDevice* task = new TaskAddDevice("foobar", "id");

  QEventLoop loop;
  connect(task, &Task::completed, [&]() { loop.exit(); });

  TaskScheduler::scheduleTask(task);
  loop.exec();
}

void TestTasks::addDevice_failure() {
  TestHelper::networkConfig.append(TestHelper::NetworkConfig(
      TestHelper::NetworkConfig::Failure, QByteArray()));

  TaskAddDevice* task = new TaskAddDevice("foobar", "id");

  QEventLoop loop;
  connect(task, &Task::completed, [&]() { loop.exit(); });

  TaskScheduler::scheduleTask(task);
  loop.exec();
}

void TestTasks::function() {
  bool completed = false;
  TaskFunction* task = new TaskFunction([&]() { completed = true; });

  TaskScheduler::scheduleTask(task);
  QVERIFY(completed);
}

void TestTasks::deletePolicy() {
  QList<int> sequence;

  TaskFunction* sentinel =
      new TaskFunction([&]() { sequence.append(99); }, Task::Reschedulable);

  TaskFunction* task = new TaskFunction(
      [&]() {
        TaskFunction* t1 =
            new TaskFunction([&]() { sequence.append(1); }, Task::Deletable);
        TaskScheduler::scheduleTask(t1);

        TaskFunction* t2 =
            new TaskFunction([&]() { sequence.append(2); }, Task::NonDeletable);
        TaskScheduler::scheduleTask(t2);

        TaskFunction* t3 = new TaskFunction([&]() { sequence.append(3); },
                                            Task::Reschedulable);
        TaskScheduler::scheduleTask(t3);

        TaskFunction* t4 =
            new TaskFunction([&]() { sequence.append(4); }, Task::NonDeletable);
        TaskScheduler::scheduleTask(t4);

        TaskFunction* t5 =
            new TaskFunction([&]() { sequence.append(5); }, Task::Deletable);
        TaskScheduler::scheduleTask(t5);

        TaskScheduler::scheduleTask(sentinel);

        TaskScheduler::deleteTasks();
      },
      Task::NonDeletable);

  QEventLoop loop;
  connect(sentinel, &Task::completed, [&]() { loop.exit(); });

  TaskScheduler::scheduleTask(task);
  loop.exec();

  // 4 tasks only because t1 and t5 are deleted.
  QCOMPARE(sequence.length(), 4);

  // The first one is t2.
  QCOMPARE(sequence.at(0), 2);

  // Then we have t4 because t3 is rescheduable.
  QCOMPARE(sequence.at(1), 4);

  // The 2 reschedulable tasks: t3 and the sentinel.
  QCOMPARE(sequence.at(2), 3);
  QCOMPARE(sequence.at(3), 99);
}

void TestTasks::deletePolicy_group() {
  // Deletable group
  {
    TaskGroup* g = new TaskGroup(
        QList<Task*>{new TaskFunction([&]() {}, Task::Deletable),
                     new TaskFunction([&]() {}, Task::Deletable)});
    QCOMPARE(g->deletePolicy(), Task::Deletable);
    delete g;
  }

  // Not deletable group
  {
    TaskGroup* g = new TaskGroup(
        QList<Task*>{new TaskFunction([&]() {}, Task::Deletable),
                     new TaskFunction([&]() {}, Task::NonDeletable),
                     new TaskFunction([&]() {}, Task::Reschedulable)});
    QCOMPARE(g->deletePolicy(), Task::NonDeletable);
    delete g;
  }

  // Reschedulable
  {
    TaskGroup* g = new TaskGroup(
        QList<Task*>{new TaskFunction([&]() {}, Task::Reschedulable),
                     new TaskFunction([&]() {}, Task::Reschedulable)});
    QCOMPARE(g->deletePolicy(), Task::Reschedulable);
    delete g;
  }

  // Not deletable group if mixed
  {
    TaskGroup* g = new TaskGroup(
        QList<Task*>{new TaskFunction([&]() {}, Task::NonDeletable),
                     new TaskFunction([&]() {}, Task::Reschedulable)});
    QCOMPARE(g->deletePolicy(), Task::NonDeletable);
    delete g;
  }
}

static TestTasks s_testTasks;
