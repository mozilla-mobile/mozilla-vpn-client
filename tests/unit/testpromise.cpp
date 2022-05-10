/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testpromise.h"
#include "../../src/promise.h"
#include "helper.h"

void TestPromise::test() {
  // Init function is immediately called.
  {
    QObject parent;
    int calls = 0;
    Promise* p = new Promise(&parent, [&](Promise::Resolver*) { ++calls; });
    QCOMPARE(calls, 1);
    QCOMPARE(p->state(), Promise::Uncompleted);
    QCOMPARE(p->resolveValues(), QVariantList());
    QCOMPARE(p->rejectValues(), QVariantList());
  }

  // Resolved without params
  {
    QObject parent;
    Promise* p =
        new Promise(&parent, [&](Promise::Resolver* r) { r->resolve(); });
    QCOMPARE(p->state(), Promise::Resolved);
    QCOMPARE(p->resolveValues(), QVariantList());
    QCOMPARE(p->rejectValues(), QVariantList());
  }

  // Resolved with params
  {
    QObject parent;
    Promise* p = new Promise(&parent, [&](Promise::Resolver* r) {
      r->resolve(42, true, QString("wow"), qApp);
    });
    QCOMPARE(p->state(), Promise::Resolved);
    QCOMPARE(p->resolveValues().size(), 4);
    QCOMPARE(p->resolveValues()[0], 42);
    QCOMPARE(p->resolveValues()[1], true);
    QCOMPARE(p->resolveValues()[2], "wow");
    QCOMPARE(qvariant_cast<QCoreApplication*>(p->resolveValues()[3]), qApp);
    QCOMPARE(p->rejectValues(), QVariantList());
  }

  // Rejected without params
  {
    QObject parent;
    Promise* p =
        new Promise(&parent, [&](Promise::Resolver* r) { r->reject(); });
    QCOMPARE(p->state(), Promise::Rejected);
    QCOMPARE(p->resolveValues(), QVariantList());
    QCOMPARE(p->rejectValues(), QVariantList());
  }

  // Reject with params
  {
    QObject parent;
    Promise* p = new Promise(&parent, [&](Promise::Resolver* r) {
      r->reject(42, true, QString("wow"), qApp);
    });
    QCOMPARE(p->state(), Promise::Rejected);
    QCOMPARE(p->resolveValues(), QVariantList());
    QCOMPARE(p->rejectValues().size(), 4);
    QCOMPARE(p->rejectValues()[0], 42);
    QCOMPARE(p->rejectValues()[1], true);
    QCOMPARE(p->rejectValues()[2], "wow");
    QCOMPARE(qvariant_cast<QCoreApplication*>(p->rejectValues()[3]), qApp);
  }

  // Chain p->then
  {
    QObject parent;
    Promise* p1 =
        new Promise(&parent, [&](Promise::Resolver* r) { r->resolve(42); });
    Promise* p2 = p1->then([](Promise::Resolver* r, const QVariantList&) {
      r->resolve(QString("wow"));
    });
    QCOMPARE(p1->state(), Promise::Resolved);
    QCOMPARE(p1->resolveValues(), QVariantList{42});
    QCOMPARE(p1->rejectValues(), QVariantList());
    QCOMPARE(p2->state(), Promise::Resolved);
    QCOMPARE(p2->resolveValues(), QVariantList{"wow"});
    QCOMPARE(p2->rejectValues(), QVariantList());
  }

  // Chain p->then(1,2)
  {
    QObject parent;
    Promise* p1 =
        new Promise(&parent, [&](Promise::Resolver* r) { r->reject(42); });
    Promise* p2 =
        p1->then([](Promise::Resolver* r,
                    const QVariantList&) { r->resolve(QString("A")); },
                 [](Promise::Resolver* r, const QVariantList&) {
                   r->resolve(QString("B"));
                 });
    QCOMPARE(p1->state(), Promise::Rejected);
    QCOMPARE(p1->resolveValues(), QVariantList());
    QCOMPARE(p1->rejectValues(), QVariantList{42});
    QCOMPARE(p2->state(), Promise::Resolved);
    QCOMPARE(p2->resolveValues(), QVariantList{"B"});
    QCOMPARE(p2->rejectValues(), QVariantList());
  }

  // Chain p->catch
  {
    QObject parent;
    Promise* p1 =
        new Promise(&parent, [&](Promise::Resolver* r) { r->reject(42); });
    Promise* p2 = p1->_catch([](Promise::Resolver* r, const QVariantList&) {
      r->resolve(QString("A"));
    });
    QCOMPARE(p1->state(), Promise::Rejected);
    QCOMPARE(p1->resolveValues(), QVariantList());
    QCOMPARE(p1->rejectValues(), QVariantList{42});
    QCOMPARE(p2->state(), Promise::Resolved);
    QCOMPARE(p2->resolveValues(), QVariantList{"A"});
    QCOMPARE(p2->rejectValues(), QVariantList());
  }

  // Chain p->then->chen
  {
    QObject parent;
    Promise* p1 =
        new Promise(&parent, [&](Promise::Resolver* r) { r->reject(42); });
    Promise* p2 = p1->then(
        [](Promise::Resolver* r, const QVariantList&) { r->resolve(43); });
    Promise* p3 = p1->_catch([](Promise::Resolver* r, const QVariantList&) {
      r->resolve(QString("B"));
    });
    QCOMPARE(p1->state(), Promise::Rejected);
    QCOMPARE(p1->resolveValues(), QVariantList());
    QCOMPARE(p1->rejectValues(), QVariantList{42});
    QCOMPARE(p2->state(), Promise::Rejected);
    QCOMPARE(p2->resolveValues(), QVariantList());
    // Error propagation!
    QCOMPARE(p2->rejectValues(), QVariantList{42});
    QCOMPARE(p3->state(), Promise::Resolved);
    QCOMPARE(p3->resolveValues(), QVariantList{"B"});
    QCOMPARE(p3->rejectValues(), QVariantList());
  }

  // Async
  {
    QObject parent;
    Promise* p = new Promise(&parent, [&](Promise::Resolver* r) {
      QTimer::singleShot(1000, [r] { r->resolve(42); });
    });

    QCOMPARE(p->state(), Promise::Uncompleted);
    QCOMPARE(p->resolveValues(), QVariantList());
    QCOMPARE(p->rejectValues(), QVariantList());

    QEventLoop loop;
    connect(p, &Promise::resolved, [&] { loop.exit(); });
    loop.exec();

    QCOMPARE(p->state(), Promise::Resolved);
    QCOMPARE(p->resolveValues(), QVariantList{42});
    QCOMPARE(p->rejectValues(), QVariantList());
  }
}

static TestPromise s_testPromise;
