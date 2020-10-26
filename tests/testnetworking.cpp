/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testnetworking.h"
#include "../src/releasemonitor.h"
#include "helper.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

// ReleaseMonitor
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void TestNetworking::releaseMonitorBasic_data()
{
    QTest::addColumn<QByteArray>("json");
    QTest::addColumn<bool>("result");

    QTest::addRow("empty") << QByteArray("") << false;
}

void TestNetworking::releaseMonitorBasic()
{
    ReleaseMonitor rm;

    QFETCH(QByteArray, json);
    QFETCH(bool, result);

    QCOMPARE(rm.processData(json), result);
}

static TestNetworking s_testNetworking;
