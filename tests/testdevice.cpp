/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testdevice.h"
#include "../src/device.h"
#include "autotest.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

void TestDevice::basic()
{
    Device device("name", QDateTime(), "publicKey", "ipv4Address", "ipv6Address");
    QCOMPARE(device.name(), "name");
    QCOMPARE(device.createdAt(), QDateTime());
    QCOMPARE(device.publicKey(), "publicKey");
    QCOMPARE(device.ipv4Address(), "ipv4Address");
    QCOMPARE(device.ipv6Address(), "ipv6Address");
    QCOMPARE(device.isDevice("name"), true);
    QCOMPARE(device.isDevice("another device"), false);
}

DECLARE_TEST(TestDevice)
