/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testbigintipv6addr.h"
#include "../../src/bigintipv6addr.h"
#include "helper.h"

namespace {
QString toBase2(const BigIntIPv6Addr& a) {
  QString x;
  for (int i = 0; i < 16; ++i) {
    QString tmp = QString::number(a.value()[i], 2);
    for (int j = tmp.length(); j < 8; ++j) x += "0";
    x += tmp;
  }
  return x;
}

}  // namespace

void TestBigIntIPv6Addr::assignOperator() {
  Q_IPV6ADDR addr1 = QHostAddress("::1").toIPv6Address();
  Q_IPV6ADDR addr2 = QHostAddress("::2").toIPv6Address();

  BigIntIPv6Addr a(addr1);
  QCOMPARE(QHostAddress(a.value()).toString(), "::1");

  BigIntIPv6Addr b(addr2);
  QCOMPARE(QHostAddress(b.value()).toString(), "::2");

  a = b;
  QCOMPARE(QHostAddress(a.value()).toString(), "::2");
}

void TestBigIntIPv6Addr::comparisonOperators() {
  Q_IPV6ADDR addr1 = QHostAddress("::1").toIPv6Address();
  Q_IPV6ADDR addr2 = QHostAddress("::2").toIPv6Address();

  // a == b
  QVERIFY(BigIntIPv6Addr(addr1) != BigIntIPv6Addr(addr2));
  QVERIFY(BigIntIPv6Addr(addr1) == BigIntIPv6Addr(addr1));
  QVERIFY(BigIntIPv6Addr(addr2) == BigIntIPv6Addr(addr2));

  // a > b
  QVERIFY(BigIntIPv6Addr(addr1) < BigIntIPv6Addr(addr2));
  QVERIFY(BigIntIPv6Addr(addr2) > BigIntIPv6Addr(addr1));

  // a >= b
  QVERIFY(BigIntIPv6Addr(addr1) <= BigIntIPv6Addr(addr2));
  QVERIFY(BigIntIPv6Addr(addr1) <= BigIntIPv6Addr(addr1));
  QVERIFY(BigIntIPv6Addr(addr2) >= BigIntIPv6Addr(addr1));
  QVERIFY(BigIntIPv6Addr(addr2) >= BigIntIPv6Addr(addr2));
}

void TestBigIntIPv6Addr::mathOperators() {
  // a++ (simple increase).
  {
    BigIntIPv6Addr a(QHostAddress("::1").toIPv6Address());
    QCOMPARE(QHostAddress((++a).value()).toString(), "::2");
  }

  // a++ (simple, 2 bytes).
  {
    BigIntIPv6Addr a(QHostAddress("::ffff").toIPv6Address());
    QCOMPARE(toBase2(a),
             "00000000000000000000000000000000000000000000000000000000000000000"
             "000000000000000000000000000000000000000000000001111111111111111");

    ++a;
    QCOMPARE(toBase2(a),
             "00000000000000000000000000000000000000000000000000000000000000000"
             "000000000000000000000000000000000000000000000010000000000000000");
    QCOMPARE(QHostAddress(a.value()).toString(), "::0.1.0.0");
  }

  // a++ (overflow).
  {
    BigIntIPv6Addr a(QHostAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff")
                         .toIPv6Address());
    QCOMPARE(toBase2(a),
             "11111111111111111111111111111111111111111111111111111111111111111"
             "111111111111111111111111111111111111111111111111111111111111111");

    ++a;
    QCOMPARE(toBase2(a),
             "00000000000000000000000000000000000000000000000000000000000000000"
             "000000000000000000000000000000000000000000000000000000000000000");
    QCOMPARE(QHostAddress(a.value()).toString(), "::");
  }

  // a += a
  {
    BigIntIPv6Addr a(QHostAddress("::1").toIPv6Address());
    a += a;
    QCOMPARE(QHostAddress(a.value()).toString(), "::2");
  }

  // a += b
  {
    BigIntIPv6Addr a(QHostAddress("::1").toIPv6Address());
    BigIntIPv6Addr b(QHostAddress("1:2:3:4:5:6:7:8").toIPv6Address());
    a += b;
    QCOMPARE(QHostAddress(a.value()).toString(), "1:2:3:4:5:6:7:9");
  }
}

void TestBigIntIPv6Addr::bitShiftOperator() {
  {
    BigIntIPv6Addr a(QHostAddress("::1").toIPv6Address());
    QCOMPARE(toBase2(a),
             "00000000000000000000000000000000000000000000000000000000000000000"
             "000000000000000000000000000000000000000000000000000000000000001");

    BigIntIPv6Addr b = a >> 1;
    QCOMPARE(toBase2(b),
             "00000000000000000000000000000000000000000000000000000000000000000"
             "000000000000000000000000000000000000000000000000000000000000000");

    QCOMPARE(QHostAddress(b.value()).toString(), "::");
  }

  {
    BigIntIPv6Addr a(QHostAddress("1::").toIPv6Address());
    QCOMPARE(toBase2(a),
             "00000000000000010000000000000000000000000000000000000000000000000"
             "000000000000000000000000000000000000000000000000000000000000000");
    BigIntIPv6Addr b = a >> 1;
    QCOMPARE(toBase2(b),
             "00000000000000001000000000000000000000000000000000000000000000000"
             "000000000000000000000000000000000000000000000000000000000000000");
    QCOMPARE(QHostAddress(b.value()).toString(), "0:8000::");
  }
}

static TestBigIntIPv6Addr s_testBigIntIPv6Addr;
