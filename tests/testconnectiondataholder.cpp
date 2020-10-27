/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testconnectiondataholder.h"
#include "../src/connectiondataholder.h"
#include "helper.h"

void TestConnectionDataHolder::checkIpAddressFailure()
{
    ConnectionDataHolder cdh;

    TestHelper::networkStatus = TestHelper::Failure;

    QEventLoop loop;
    connect(&cdh, &ConnectionDataHolder::ipAddressChecked, [&] {
        cdh.disable();
        loop.exit();
    }); 

    cdh.enable();
    loop.exec();
}

void TestConnectionDataHolder::checkIpAddressSucceess_data()
{
    QTest::addColumn<QByteArray>("json");
    QTest::addColumn<QString>("ipAddress");

    QTest::addRow("invalid") << QByteArray("") << "vpn.connectionInfo.unknown";

    QJsonObject json;
    QTest::addRow("empty") << QJsonDocument(json).toJson() << "vpn.connectionInfo.unknown";

    json.insert("ip", 42);
    QTest::addRow("invalid ip") << QJsonDocument(json).toJson() << "vpn.connectionInfo.unknown";

    json.insert("ip", "42");
    QTest::addRow("valid ip") << QJsonDocument(json).toJson() << "42";
}

void TestConnectionDataHolder::checkIpAddressSucceess()
{
    ConnectionDataHolder cdh;

    TestHelper::networkStatus = TestHelper::Success;

    QFETCH(QByteArray, json);
    TestHelper::networkBody = json;

    QEventLoop loop;
    connect(&cdh, &ConnectionDataHolder::ipAddressChecked, [&] {
        cdh.disable();

        QFETCH(QString, ipAddress);
        QCOMPARE(cdh.ipAddress(), ipAddress);

        loop.exit();
    }); 

    cdh.enable();
    loop.exec();
}

static TestConnectionDataHolder s_testConnectionDataHolder;
