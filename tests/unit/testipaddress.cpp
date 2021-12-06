/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testipaddress.h"
#include "../../src/ipaddress.h"
#include "helper.h"

void TestIpAddress::ctor() {
  IPAddress ip;
  QCOMPARE(ip, ip);
  ip = ip;
}

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
  QTest::addRow("::1") << "::1"
                       << "::1" << 128
                       << "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"
                       << "::"
                       << "::1";
  QTest::addRow("::1/128") << "::1/128"
                           << "::1" << 128
                           << "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"
                           << "::"
                           << "::1";

  QTest::addRow("random v4") << "1.1.1.1"
                             << "1.1.1.1" << 32 << "255.255.255.255"
                             << "0.0.0.0"
                             << "1.1.1.1";

  QTest::addRow("random v6")
      << "1:2:3:4:5:6:7:8"
      << "1:2:3:4:5:6:7:8" << 128 << "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"
      << "::"
      << "1:2:3:4:5:6:7:8";

  QTest::addRow("world v4") << "0.0.0.0/0"
                            << "0.0.0.0" << 0 << "0.0.0.0"
                            << "255.255.255.255"
                            << "255.255.255.255";

  QTest::addRow("world v6") << "::/0"
                            << "::" << 0 << "::"
                            << "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"
                            << "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff";

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

  QTest::addRow("netmask/87 v6")
      << "1:2:3:4:5::/87"
      << "1:2:3:4:5::" << 87 << "ffff:ffff:ffff:ffff:ffff:fe00::"
      << "::1ff:ffff:ffff"
      << "1:2:3:4:5:1ff:ffff:ffff";
}

void TestIpAddress::basic() {
  QFETCH(QString, input);

  IPAddress ipAddress = IPAddress(input);

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
  QTest::addRow("self ::1") << "::1"
                            << "::1" << true;
  QTest::addRow("world v4") << "0.0.0.0/0"
                            << "127.0.0.1" << true;
  QTest::addRow("world v6") << "::/0"
                            << "::1" << true;
  QTest::addRow("A4") << "1.2.3.0/24"
                      << "127.0.0.1" << false;
  QTest::addRow("B4") << "1.2.3.0/24"
                      << "1.2.2.0/24" << false;
  QTest::addRow("C4") << "1.2.3.0/24"
                      << "1.2.4.3" << false;
  QTest::addRow("D6") << "1:2:3:4:5::/87"
                      << "::1" << false;
  QTest::addRow("E6") << "1:2:3:4:5::/87"
                      << "1:2:3:4:5:6:7:8" << true;
  QTest::addRow("F6") << "1:2:3:4:5::/87"
                      << "1:2:3:5:5:6:7:8" << false;
}

void TestIpAddress::overlaps() {
  QFETCH(QString, a);
  IPAddress ipAddressA = IPAddress(a);

  QFETCH(QString, b);
  IPAddress ipAddressB = IPAddress(b);

  QFETCH(bool, result);
  QCOMPARE(ipAddressA.overlaps(ipAddressB), result);
}

void TestIpAddress::contains_data() {
  QTest::addColumn<QString>("a");
  QTest::addColumn<QString>("b");
  QTest::addColumn<bool>("result");

  QTest::addRow("self localhost") << "127.0.0.1"
                                  << "127.0.0.1" << true;
  QTest::addRow("self ::1") << "::1"
                            << "::1" << true;
  QTest::addRow("world v4") << "0.0.0.0/0"
                            << "127.0.0.1" << true;
  QTest::addRow("world v6") << "::/0"
                            << "::1" << true;
  QTest::addRow("A4") << "1.2.3.0/24"
                      << "127.0.0.1" << false;
  QTest::addRow("B4") << "1.2.3.0/24"
                      << "1.2.2.0/24" << false;
  QTest::addRow("C4") << "1.2.3.0/24"
                      << "1.2.4.3" << false;
  QTest::addRow("D6") << "1:2:3:4:5::/87"
                      << "::1" << false;
  QTest::addRow("E4") << "1:2:3:4:5::/87"
                      << "1:2:3:4:5:6:7:8" << true;
  QTest::addRow("F4") << "1:2:3:4:5::/87"
                      << "1:2:3:5:6:7:8:9" << false;
}

void TestIpAddress::contains() {
  QFETCH(QString, a);
  IPAddress ipAddressA = IPAddress(a);

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
  QTest::addRow("self ::1") << "::1"
                            << "::1" << true;
  QTest::addRow("world vs localhost v4") << "0.0.0.0/0"
                                         << "127.0.0.1" << false;
  QTest::addRow("world vs localhost v6") << "::/0"
                                         << "::1" << false;
  QTest::addRow("world vs world v4") << "0.0.0.0/0"
                                     << "0.0.0.0/0" << true;
  QTest::addRow("world vs world v6") << "::/0"
                                     << "::/0" << true;
}

void TestIpAddress::equal() {
  QFETCH(QString, a);
  IPAddress ipAddressA = IPAddress(a);

  QFETCH(QString, b);
  IPAddress ipAddressB = IPAddress(b);

  QFETCH(bool, result);
  QCOMPARE(ipAddressA == ipAddressB, result);
}

void TestIpAddress::excludeAddresses_data() {
  QTest::addColumn<QString>("input");
  QTest::addColumn<QString>("excludeAddresses");
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

  QTest::addRow("world vs ::1")
      << "::/0"
      << "::1"
      << "0:0:0:1000::/52,0:0:0:100::/56,0:0:0:10::/60,0:0:0:1::/"
         "64,0:0:0:2000::/51,0:0:0:200::/55,0:0:0:20::/59,0:0:0:2::/"
         "63,0:0:0:4000::/50,0:0:0:400::/54,0:0:0:40::/58,0:0:0:4::/"
         "62,0:0:0:8000::/49,0:0:0:800::/53,0:0:0:80::/57,0:0:0:8::/"
         "61,0:0:1000::/36,0:0:100::/40,0:0:10::/44,0:0:1::/48,0:0:2000::/"
         "35,0:0:200::/39,0:0:20::/43,0:0:2::/47,0:0:4000::/34,0:0:400::/"
         "38,0:0:40::/42,0:0:4::/46,0:0:8000::/33,0:0:800::/37,0:0:80::/"
         "41,0:0:8::/45,0:1000::/20,0:100::/24,0:10::/28,0:1::/32,0:2000::/"
         "19,0:200::/23,0:20::/27,0:2::/31,0:4000::/18,0:400::/22,0:40::/"
         "26,0:4::/30,0:8000::/17,0:800::/21,0:80::/25,0:8::/29,1000::/4,100::/"
         "8,10::/12,1::/16,2000::/3,200::/7,20::/11,2::/15,4000::/2,400::/"
         "6,40::/10,4::/14,8000::/1,800::/5,80::/9,8::/13,::/128,::0.1.0.0/"
         "112,::0.128.0.0/105,::0.16.0.0/108,::0.2.0.0/111,::0.32.0.0/"
         "107,::0.4.0.0/110,::0.64.0.0/106,::0.8.0.0/109,::1.0.0.0/104,::10/"
         "124,::100/120,::1000/116,::1000:0:0/84,::1000:0:0:0/68,::100:0:0/"
         "88,::100:0:0:0/72,::10:0:0/92,::10:0:0:0/76,::128.0.0.0/"
         "97,::16.0.0.0/100,::1:0:0/96,::1:0:0:0/80,::2.0.0.0/103,::2/127,::20/"
         "123,::200/119,::2000/115,::2000:0:0/83,::2000:0:0:0/67,::200:0:0/"
         "87,::200:0:0:0/71,::20:0:0/91,::20:0:0:0/75,::2:0:0/95,::2:0:0:0/"
         "79,::32.0.0.0/99,::4.0.0.0/102,::4/126,::40/122,::400/118,::4000/"
         "114,::4000:0:0/82,::4000:0:0:0/66,::400:0:0/86,::400:0:0:0/"
         "70,::40:0:0/90,::40:0:0:0/74,::4:0:0/94,::4:0:0:0/78,::64.0.0.0/"
         "98,::8.0.0.0/101,::8/125,::80/121,::800/117,::8000/113,::8000:0:0/"
         "81,::8000:0:0:0/65,::800:0:0/85,::800:0:0:0/69,::80:0:0/"
         "89,::80:0:0:0/73,::8:0:0/93,::8:0:0:0/77";
}

void TestIpAddress::excludeAddresses() {
  QFETCH(QString, input);
  IPAddress a = IPAddress(input);

  QFETCH(QString, excludeAddresses);
  IPAddress b = IPAddress(excludeAddresses);

  QStringList list;
  for (const IPAddress& r : a.excludeAddresses(b)) {
    list.append(r.toString());
  }

  std::sort(list.begin(), list.end());

  QFETCH(QString, result);
  qDebug() << list.join(",");
  QVERIFY(list.join(",") == result);
}

static TestIpAddress s_testIpAddress;
