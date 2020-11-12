/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testtimersingleshot.h"
#include "../../src/timersingleshot.h"

#include <QTimer>

void TestTimerSingleShot::basic()
{
    {
        bool called = false;
        TimerSingleShot::create(nullptr, 0, [&] { called = true; });

        QEventLoop loop;
        QTimer t;
        connect(&t, &QTimer::timeout, [&] { loop.exit(); });
        t.start(0);
        loop.exec();

        QVERIFY(called);
    }

    {
        QTimer* obj = new QTimer();

        bool called = false;
        TimerSingleShot::create(obj, 0, [&] { called = true; });

        QEventLoop loop;
        QTimer t;
        connect(&t, &QTimer::timeout, [&] { loop.exit(); });
        t.start(0);
        loop.exec();

        QVERIFY(called);
        delete obj;
    }

    {
        QTimer* obj = new QTimer();

        bool called = false;
        TimerSingleShot::create(obj, 0, [&] { called = true; });
        delete obj;

        QEventLoop loop;
        QTimer t;
        connect(&t, &QTimer::timeout, [&] { loop.exit(); });
        t.start(0);
        loop.exec();

        QVERIFY(!called);
    }
}

static TestTimerSingleShot s_testTimerSingleShot;
