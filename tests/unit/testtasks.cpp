/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testtasks.h"
#include "../../src/core.h"
#include "../../src/tasks/accountandservers/taskaccountandservers.h"
#include "../../src/tasks/adddevice/taskadddevice.h"
#include "../../src/tasks/function/taskfunction.h"

void TestTasks::accountAndServers() {
  // Failure
  {
    TestHelper::networkConfig.append(TestHelper::NetworkConfig(
        TestHelper::NetworkConfig::Failure, QByteArray()));
    TestHelper::networkConfig.append(TestHelper::NetworkConfig(
        TestHelper::NetworkConfig::Failure, QByteArray()));

    TaskAccountAndServers* task = new TaskAccountAndServers();

    QEventLoop loop;
    connect(task, &Task::completed, [&]() { loop.exit(); });

    Core::instance()->scheduleTask(task);
    loop.exec();
  }

  // Success
  {
    TestHelper::networkConfig.append(TestHelper::NetworkConfig(
        TestHelper::NetworkConfig::Success, QByteArray()));
    TestHelper::networkConfig.append(TestHelper::NetworkConfig(
        TestHelper::NetworkConfig::Success, QByteArray()));

    TaskAccountAndServers* task = new TaskAccountAndServers();

    QEventLoop loop;
    connect(task, &Task::completed, [&]() { loop.exit(); });

    Core::instance()->scheduleTask(task);
    loop.exec();
  }
}

void TestTasks::addDevice_success() {
  TestHelper::networkConfig.append(TestHelper::NetworkConfig(
      TestHelper::NetworkConfig::Success, QByteArray()));

  TaskAddDevice* task = new TaskAddDevice("foobar");

  QEventLoop loop;
  connect(task, &Task::completed, [&]() { loop.exit(); });

  Core::instance()->scheduleTask(task);
  loop.exec();
}

void TestTasks::addDevice_failure() {
  TestHelper::networkConfig.append(TestHelper::NetworkConfig(
      TestHelper::NetworkConfig::Failure, QByteArray()));

  TaskAddDevice* task = new TaskAddDevice("foobar");

  QEventLoop loop;
  connect(task, &Task::completed, [&]() { loop.exit(); });

  Core::instance()->scheduleTask(task);
  loop.exec();
}

void TestTasks::authenticate() {
  // TODO
}

void TestTasks::function() {
  bool completed = false;
  TaskFunction* task = new TaskFunction([&](Core* core) {
    completed = true;
    QCOMPARE(core, Core::instance());
  });

  Core::instance()->scheduleTask(task);
  QVERIFY(completed);
}

void TestTasks::removeDevice() {
  // TODO
}

static TestTasks s_testTasks;
