/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testtasks.h"
#include "../../src/mozillavpn.h"
#include "../../src/tasks/account/taskaccount.h"
#include "../../src/tasks/adddevice/taskadddevice.h"
#include "../../src/tasks/function/taskfunction.h"
#include "../../src/tasks/servers/taskservers.h"
#include "../../src/taskscheduler.h"

void TestTasks::account() {
  // Failure
  {
    TestHelper::networkConfig.append(TestHelper::NetworkConfig(
        TestHelper::NetworkConfig::Failure, QByteArray()));

    TaskAccount* task = new TaskAccount();

    QEventLoop loop;
    connect(task, &Task::completed, [&]() { loop.exit(); });

    TaskScheduler::scheduleTask(task);
    loop.exec();
  }

  // Success
  {
    TestHelper::networkConfig.append(TestHelper::NetworkConfig(
        TestHelper::NetworkConfig::Success, QByteArray()));

    TaskAccount* task = new TaskAccount();

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

    TaskServers* task = new TaskServers();

    QEventLoop loop;
    connect(task, &Task::completed, [&]() { loop.exit(); });

    TaskScheduler::scheduleTask(task);
    loop.exec();
  }

  // Success
  {
    TestHelper::networkConfig.append(TestHelper::NetworkConfig(
        TestHelper::NetworkConfig::Success, QByteArray()));

    TaskServers* task = new TaskServers();

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

void TestTasks::removeDevice() {
  // TODO
}

static TestTasks s_testTasks;
