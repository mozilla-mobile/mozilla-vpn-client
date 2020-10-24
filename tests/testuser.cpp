/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testuser.h"
#include "../src/user.h"
#include "autotest.h"

void TestUser::fromJson_data()
{
    QTest::addColumn<QByteArray>("json");
    QTest::addColumn<bool>("result");
    QTest::addColumn<QString>("avatar");
    QTest::addColumn<QString>("displayName");
    QTest::addColumn<QString>("email");
    QTest::addColumn<int>("maxDevices");
    QTest::addColumn<bool>("subscriptionNeeded");

    QTest::newRow("null") << QByteArray("") << false << ""
                          << ""
                          << "" << 5 << false;
    QTest::newRow("invalid") << QByteArray("wow") << false << ""
                             << ""
                             << "" << 5 << false;
    QTest::newRow("array") << QByteArray("[]") << false << ""
                           << ""
                           << "" << 5 << false;

    QJsonObject obj;

    QTest::newRow("empty object") << QJsonDocument(obj).toJson() << false << ""
                                  << ""
                                  << "" << 5 << false;

    obj.insert("a", QJsonValue("b"));
    QTest::newRow("no avatar") << QJsonDocument(obj).toJson() << false << ""
                               << ""
                               << "" << 5 << false;

    obj.insert("avatar", QJsonValue("avatar"));
    QTest::newRow("no displayName") << QJsonDocument(obj).toJson() << false << "avatar"
                                    << ""
                                    << "" << 5 << false;

    obj.insert("display_name", QJsonValue("displayName"));
    QTest::newRow("no email") << QJsonDocument(obj).toJson() << false << "avatar"
                              << "displayName"
                              << "" << 5 << false;

    obj.insert("email", QJsonValue("email"));
    QTest::newRow("no maxDevices") << QJsonDocument(obj).toJson() << false << "avatar"
                                   << "displayName"
                                   << "email" << 5 << false;

    obj.insert("max_devices", QJsonValue(123));
    QTest::newRow("no maxDevices") << QJsonDocument(obj).toJson() << false << "avatar"
                                   << "displayName"
                                   << "email" << 123 << false;

    obj.insert("subscriptions", QJsonValue("wow"));
    QTest::newRow("invalid subscription") << QJsonDocument(obj).toJson() << false << "avatar"
                                          << "displayName"
                                          << "email" << 123 << false;

    QJsonObject subscription;
    obj.insert("subscriptions", subscription);
    QTest::newRow("empty subscription") << QJsonDocument(obj).toJson() << true << "avatar"
                                        << "displayName"
                                        << "email" << 123 << true;

    subscription.insert("vpn", QJsonValue("WOW"));
    obj.insert("subscriptions", subscription);
    QTest::newRow("invalid vpn subscription") << QJsonDocument(obj).toJson() << false << "avatar"
                                              << "displayName"
                                              << "email" << 123 << true;

    QJsonObject subVpn;
    subscription.insert("vpn", subVpn);
    obj.insert("subscriptions", subscription);
    QTest::newRow("empty vpn subscription") << QJsonDocument(obj).toJson() << false << "avatar"
                                            << "displayName"
                                            << "email" << 123 << true;

    subVpn.insert("active", QJsonValue("sure!"));
    subscription.insert("vpn", subVpn);
    obj.insert("subscriptions", subscription);
    QTest::newRow("invalid active vpn subscription")
        << QJsonDocument(obj).toJson() << false << "avatar"
        << "displayName"
        << "email" << 123 << true;

    subVpn.insert("active", QJsonValue(true));
    subscription.insert("vpn", subVpn);
    obj.insert("subscriptions", subscription);
    QTest::newRow("active vpn subscription") << QJsonDocument(obj).toJson() << true << "avatar"
                                             << "displayName"
                                             << "email" << 123 << false;

    subVpn.insert("active", QJsonValue(false));
    subscription.insert("vpn", subVpn);
    obj.insert("subscriptions", subscription);
    QTest::newRow("inactive vpn subscription") << QJsonDocument(obj).toJson() << true << "avatar"
                                               << "displayName"
                                               << "email" << 123 << true;
}

void TestUser::fromJson()
{
    QFETCH(QByteArray, json);
    QFETCH(bool, result);
    QFETCH(QString, avatar);
    QFETCH(QString, displayName);
    QFETCH(QString, email);
    QFETCH(int, maxDevices);
    QFETCH(bool, subscriptionNeeded);

    User user;
    QCOMPARE(user.fromJson(json), result);

    QCOMPARE(user.avatar(), avatar);
    QCOMPARE(user.displayName(), displayName);
    QCOMPARE(user.email(), email);
    QCOMPARE(user.maxDevices(), maxDevices);
    QCOMPARE(user.subscriptionNeeded(), subscriptionNeeded);
}

DECLARE_TEST(TestUser)
