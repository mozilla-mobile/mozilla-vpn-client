/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testtasks.h"

#include <QTimer>

#include "networking/simplenetworkmanager.h"
#include "settings/settingsholder.h"
#include "taskfunction.h"
#include "taskgroup.h"
#include "taskscheduler.h"

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
  connect(sentinel, &Task::completed, sentinel, [&]() { loop.exit(); });

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

void TestTasks::deletePolicy_async() {
  class TaskAsync final : public Task {
   public:
    TaskAsync(Task::DeletePolicy deletePolicy)
        : Task("TaskAsync"), m_deletePolicy(deletePolicy) {}

    void run() override { QTimer::singleShot(500, this, &Task::completed); }

    DeletePolicy deletePolicy() const override { return m_deletePolicy; }

   private:
    DeletePolicy m_deletePolicy = Deletable;
  };

  Task* t1 = new TaskAsync(Task::Deletable);
  TaskScheduler::scheduleTask(t1);

  Task* t2 = new TaskAsync(Task::NonDeletable);
  TaskScheduler::scheduleTask(t2);

  QEventLoop loop;
  connect(t2, &Task::completed, [&]() { loop.exit(); });

  TaskScheduler::deleteTasks();
  loop.exec();
}

void TestTasks::deleteTasks() {
  QStringList sequence;

  class TaskAsync final : public Task {
   public:
    TaskAsync(Task::DeletePolicy deletePolicy, QStringList* sequence,
              const QString& name)
        : Task("TaskAsync"),
          m_deletePolicy(deletePolicy),
          m_sequence(sequence),
          m_name(name) {}

    void run() override {
      QTimer::singleShot(500, this, [this]() {
        m_sequence->append(m_name);
        emit completed();
      });
    }

    DeletePolicy deletePolicy() const override { return m_deletePolicy; }

   private:
    DeletePolicy m_deletePolicy = Deletable;
    QStringList* m_sequence = nullptr;
    QString m_name;
  };

  Task* t1 = new TaskAsync(Task::Deletable, &sequence, "t1");
  TaskScheduler::scheduleTask(t1);

  Task* t2 = new TaskAsync(Task::NonDeletable, &sequence, "t2");
  TaskScheduler::scheduleTask(t2);

  TaskScheduler::deleteTasks();

  Task* t3 = new TaskAsync(Task::NonDeletable, &sequence, "t3");

  QEventLoop loop;
  connect(t3, &Task::completed, [&]() { loop.exit(); });

  TaskScheduler::scheduleTask(t3);
  loop.exec();

  QCOMPARE(sequence.length(), 2);
  QCOMPARE(sequence.at(0), "t2");
  QCOMPARE(sequence.at(1), "t3");
}

void TestTasks::forceDeleteTasks() {
  QStringList sequence;

  class TaskAsync final : public Task {
   public:
    TaskAsync(Task::DeletePolicy deletePolicy, QStringList* sequence,
              const QString& name)
        : Task("TaskAsync"),
          m_deletePolicy(deletePolicy),
          m_sequence(sequence),
          m_name(name) {}

    void run() override {
      QTimer::singleShot(500, this, [this]() {
        m_sequence->append(m_name);
        emit completed();
      });
    }

    DeletePolicy deletePolicy() const override { return m_deletePolicy; }

   private:
    DeletePolicy m_deletePolicy = Deletable;
    QStringList* m_sequence = nullptr;
    QString m_name;
  };

  Task* t1 = new TaskAsync(Task::Deletable, &sequence, "t1");
  TaskScheduler::scheduleTask(t1);

  Task* t2 = new TaskAsync(Task::NonDeletable, &sequence, "t2");
  TaskScheduler::scheduleTask(t2);

  TaskScheduler::forceDeleteTasks();

  Task* t3 = new TaskAsync(Task::NonDeletable, &sequence, "t3");

  QEventLoop loop;
  connect(t3, &Task::completed, [&]() { loop.exit(); });
  TaskScheduler::scheduleTask(t3);
  loop.exec();

  QCOMPARE(sequence.length(), 1);
  QCOMPARE(sequence.at(0), "t3");
}

QTEST_MAIN(TestTasks);
