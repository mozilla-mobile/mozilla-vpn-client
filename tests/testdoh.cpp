/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testdoh.h"
#include "../src/dohrequest.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

void TestDOH::basic_data()
{
    QTest::addColumn<QByteArray>("json");
    QTest::addColumn<bool>("result");
    QTest::addColumn<QStringList>("ipv4Addresses");
    QTest::addColumn<QStringList>("ipv6Addresses");

    QTest::addRow("failure") << QByteArray() << false << QStringList() << QStringList();

    QTest::addRow("invalid") << QByteArray() << true << QStringList() << QStringList();

    QJsonObject json;
    QTest::addRow("empty") << QJsonDocument(json).toJson() << true << QStringList()
                           << QStringList();

    json.insert("Status", 42);
    QTest::addRow("error status") << QJsonDocument(json).toJson() << true << QStringList()
                                  << QStringList();

    json.insert("Status", 0);
    json.insert("Answer", 42);
    QTest::addRow("invalid answer")
        << QJsonDocument(json).toJson() << true << QStringList() << QStringList();

    QJsonArray answers;
    json.insert("Answer", answers);
    QTest::addRow("empty answer") << QJsonDocument(json).toJson() << true << QStringList()
                                  << QStringList();

    answers.append(42);
    json.insert("Answer", answers);
    QTest::addRow("invalid answer object")
        << QJsonDocument(json).toJson() << true << QStringList() << QStringList();

    QJsonObject answer;
    answers.replace(0, answer);
    json.insert("Answer", answers);
    QTest::addRow("empty answer object")
        << QJsonDocument(json).toJson() << true << QStringList() << QStringList();

    answer.insert("type", 42);
    answers.replace(0, answer);
    json.insert("Answer", answers);
    QTest::addRow("invalid answer object type")
        << QJsonDocument(json).toJson() << true << QStringList() << QStringList();

    answer.insert("type", 1);
    answers.replace(0, answer);
    json.insert("Answer", answers);
    QTest::addRow("invalid answer object type")
        << QJsonDocument(json).toJson() << true << QStringList() << QStringList();

    answer.insert("data", "ipv4");
    answers.replace(0, answer);
    json.insert("Answer", answers);
    QTest::addRow("ipv4") << QJsonDocument(json).toJson() << true << QStringList{"ipv4"}
                          << QStringList();

    answer.insert("type", 28);
    answer.insert("data", "ipv6");
    answers.replace(0, answer);
    json.insert("Answer", answers);
    QTest::addRow("ipv6") << QJsonDocument(json).toJson() << true << QStringList() << QStringList{"ipv6"};
}

void TestDOH::basic()
{
    QFETCH(QByteArray, json);
    QFETCH(bool, result);

    if (result) {
        // ipv4:
        TestHelper::networkConfig.append(
            TestHelper::NetworkConfig(TestHelper::NetworkConfig::Success, json));
        // ipv6:
        TestHelper::networkConfig.append(
            TestHelper::NetworkConfig(TestHelper::NetworkConfig::Failure, QByteArray()));
    } else {
        // ipv4:
        TestHelper::networkConfig.append(
            TestHelper::NetworkConfig(TestHelper::NetworkConfig::Failure, QByteArray()));
        // ipv6:
        TestHelper::networkConfig.append(
            TestHelper::NetworkConfig(TestHelper::NetworkConfig::Failure, QByteArray()));
    }

    DOHRequest *r = new DOHRequest(this);
    r->resolve("foobar");

    bool completed = false;

    QEventLoop loop;
    connect(r,
            &DOHRequest::completed,
            [&](const QStringList &resultIpv4Addresses, const QStringList &resultIpv6Addresses) {
                QFETCH(QStringList, ipv4Addresses);
                QCOMPARE(ipv4Addresses, resultIpv4Addresses);

                QFETCH(QStringList, ipv6Addresses);
                QCOMPARE(ipv6Addresses, resultIpv6Addresses);

                completed = true;
            });

    connect(r, &DOHRequest::destroyed, [&]() {
        QVERIFY(completed);
        loop.exit();
    });
    loop.exec();
}

static TestDOH s_testDOH;
