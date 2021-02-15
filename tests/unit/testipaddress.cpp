/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testipaddress.h"
#include "../../src/ipaddress.h"
#include "helper.h"

void TestIpAddress::basic_data() {
  QTest::addColumn<QString>("input");
  QTest::addColumn<QString>("address");
  QTest::addColumn<int>("prefixLength");
  QTest::addColumn<QString>("netmask");
  QTest::addColumn<QString>("hostmask");
  QTest::addColumn<QString>("broadcastAddress");

  QTest::addRow("localhost") << "127.0.0.1"
                             << "127.0.0.1" << 32 << "255.255.255.255"
                             << "0.0.0.0"
                             << "127.0.0.1";
  QTest::addRow("localhost/32") << "127.0.0.1/32"
                                << "127.0.0.1" << 32 << "255.255.255.255"
                                << "0.0.0.0"
                                << "127.0.0.1";

  QTest::addRow("random") << "1.1.1.1"
                          << "1.1.1.1" << 32 << "255.255.255.255"
                          << "0.0.0.0"
                          << "1.1.1.1";

  QTest::addRow("world") << "0.0.0.0/0"
                         << "0.0.0.0" << 0 << "0.0.0.0"
                         << "255.255.255.255"
                         << "255.255.255.255";

  QTest::addRow("netmask C") << "192.168.1.0/24"
                             << "192.168.1.0" << 24 << "255.255.255.0"
                             << "0.0.0.255"
                             << "192.168.1.255";

  QTest::addRow("netmask/8") << "192.0.0.0/8"
                             << "192.0.0.0" << 8 << "255.0.0.0"
                             << "0.255.255.255"
                             << "192.255.255.255";
  QTest::addRow("netmask/30") << "192.0.0.0/30"
                              << "192.0.0.0" << 30 << "255.255.255.252"
                              << "0.0.0.3"
                              << "192.0.0.3";
}

void TestIpAddress::basic() {
  QFETCH(QString, input);

  IPAddress ipAddress = IPAddress::create(input);

  QFETCH(QString, address);
  QCOMPARE(ipAddress.address().toString(), address);
  QFETCH(int, prefixLength);
  QCOMPARE(ipAddress.prefixLength(), prefixLength);
  QFETCH(QString, netmask);
  QCOMPARE(ipAddress.netmask().toString(), netmask);
  QFETCH(QString, hostmask);
  QCOMPARE(ipAddress.hostmask().toString(), hostmask);
  QFETCH(QString, broadcastAddress);
  QCOMPARE(ipAddress.broadcastAddress().toString(), broadcastAddress);
}

void TestIpAddress::overlaps_data() {
  QTest::addColumn<QString>("a");
  QTest::addColumn<QString>("b");
  QTest::addColumn<bool>("result");

  QTest::addRow("self localhost") << "127.0.0.1"
                                  << "127.0.0.1" << true;
  QTest::addRow("world") << "0.0.0.0/0"
                         << "127.0.0.1" << true;
  QTest::addRow("A") << "1.2.3.0/24"
                     << "127.0.0.1" << false;
  QTest::addRow("B") << "1.2.3.0/24"
                     << "1.2.2.0/24" << false;
  QTest::addRow("C") << "1.2.3.0/24"
                     << "1.2.4.3" << false;
}

void TestIpAddress::overlaps() {
  QFETCH(QString, a);
  IPAddress ipAddressA = IPAddress::create(a);

  QFETCH(QString, b);
  IPAddress ipAddressB = IPAddress::create(b);

  QFETCH(bool, result);
  QCOMPARE(ipAddressA.overlaps(ipAddressB), result);
}

void TestIpAddress::contains_data() {
  QTest::addColumn<QString>("a");
  QTest::addColumn<QString>("b");
  QTest::addColumn<bool>("result");

  QTest::addRow("self localhost") << "127.0.0.1"
                                  << "127.0.0.1" << true;
  QTest::addRow("world") << "0.0.0.0/0"
                         << "127.0.0.1" << true;
  QTest::addRow("A") << "1.2.3.0/24"
                     << "127.0.0.1" << false;
  QTest::addRow("B") << "1.2.3.0/24"
                     << "1.2.2.0/24" << false;
  QTest::addRow("C") << "1.2.3.0/24"
                     << "1.2.4.3" << false;
}

void TestIpAddress::contains() {
  QFETCH(QString, a);
  IPAddress ipAddressA = IPAddress::create(a);

  QFETCH(QString, b);
  QHostAddress ipAddressB(b);

  QFETCH(bool, result);
  QCOMPARE(ipAddressA.contains(ipAddressB), result);
}

void TestIpAddress::equal_data() {
  QTest::addColumn<QString>("a");
  QTest::addColumn<QString>("b");
  QTest::addColumn<bool>("result");

  QTest::addRow("self localhost") << "127.0.0.1"
                                  << "127.0.0.1" << true;
  QTest::addRow("world vs localhost") << "0.0.0.0/0"
                                      << "127.0.0.1" << false;
  QTest::addRow("world vs world") << "0.0.0.0/0"
                                  << "0.0.0.0/0" << true;
}

void TestIpAddress::equal() {
  QFETCH(QString, a);
  IPAddress ipAddressA = IPAddress::create(a);

  QFETCH(QString, b);
  IPAddress ipAddressB = IPAddress::create(b);

  QFETCH(bool, result);
  QCOMPARE(ipAddressA == ipAddressB, result);
}

void TestIpAddress::excludes_data() {
  QTest::addColumn<QString>("input");
  QTest::addColumn<QString>("excludes");
  QTest::addColumn<QString>("result");

  QTest::addRow("world vs localhost")
      << "0.0.0.0/0"
      << "127.0.0.1"
      << "0.0.0.0/2,112.0.0.0/5,120.0.0.0/6,124.0.0.0/7,126.0.0.0/8,127.0.0.0/"
         "32,127.0.0.128/25,127.0.0.16/28,127.0.0.2/31,127.0.0.32/27,127.0.0.4/"
         "30,127.0.0.64/26,127.0.0.8/29,127.0.1.0/24,127.0.128.0/17,127.0.16.0/"
         "20,127.0.2.0/23,127.0.32.0/19,127.0.4.0/22,127.0.64.0/18,127.0.8.0/"
         "21,127.1.0.0/16,127.128.0.0/9,127.16.0.0/12,127.2.0.0/15,127.32.0.0/"
         "11,127.4.0.0/14,127.64.0.0/10,127.8.0.0/13,128.0.0.0/1,64.0.0.0/"
         "3,96.0.0.0/4";

  QTest::addRow("world vs rfc1918 (part)")
      << "0.0.0.0/0"
      << "10.0.0.0/8"
      << "0.0.0.0/5,11.0.0.0/8,12.0.0.0/6,128.0.0.0/1,16.0.0.0/4,32.0.0.0/"
         "3,64.0.0.0/2,8.0.0.0/7";
}

void TestIpAddress::excludes() {
  QFETCH(QString, input);
  IPAddress a = IPAddress::create(input);

  QFETCH(QString, excludes);
  IPAddress b = IPAddress::create(excludes);

  QStringList list;
  for (const IPAddress& r : a.excludes(b)) {
    list.append(r.toString());
  }

  std::sort(list.begin(), list.end());

  QFETCH(QString, result);
  QVERIFY(list.join(",") == result);
}

static TestIpAddress s_testIpAddress;
