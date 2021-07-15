/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testbigint.h"
#include "../../src/bigint.h"
#include "helper.h"

namespace {
QString toBase2(const BigInt& a) {
  QString x;
  for (int i = 0; i < a.size(); ++i) {
    QString tmp = QString::number(a.value()[i], 2);
    for (int j = tmp.length(); j < 8; ++j) x += "0";
    x += tmp;
  }
  return x;
}

}  // namespace

void TestBigInt::ctor() {
  {
    BigInt a(1);
    QCOMPARE(toBase2(a), "00000000");
  }
  {
    BigInt a(2);
    QCOMPARE(toBase2(a), "0000000000000000");
  }
}

void TestBigInt::mathOperators() {
  // a++ (simple increase).
  {
    BigInt a(2);
    QCOMPARE(toBase2(++a), "0000000000000001");
  }

  // a++ (simple, 2 bytes).
  {
    BigInt a(2);
    for (int i = 0; i < 255; ++i) ++a;
    QCOMPARE(toBase2(a), "0000000011111111");
    QCOMPARE(toBase2(++a), "0000000100000000");
  }

  // a++ (overflow).
  {
    BigInt a(1);
    for (int i = 0; i < 255; ++i) ++a;
    QCOMPARE(toBase2(a), "11111111");
    QCOMPARE(toBase2(++a), "00000000");
  }

  // a += a
  {
    BigInt a(2);
    ++a;
    a += a;
    QCOMPARE(toBase2(a), "0000000000000010");
  }

  // a += b
  {
    BigInt a(2);
    for (int i = 0; i < 255; ++i) ++a;
    QCOMPARE(toBase2(a), "0000000011111111");

    BigInt b(2);
    ++b;

    a += b;
    QCOMPARE(toBase2(a), "0000000100000000");
  }
}

void TestBigInt::comparisonOperators() {
  BigInt a(2);
  ++a;

  BigInt b(2);
  ++b;
  ++b;

  // a == b
  QVERIFY(a != b);
  QVERIFY(a == a);
  QVERIFY(b == b);

  // a > b
  QVERIFY(a < b);
  QVERIFY(b > a);

  // a >= b
  QVERIFY(a <= b);
  QVERIFY(a <= a);
  QVERIFY(b >= a);
  QVERIFY(b >= b);
}

void TestBigInt::bitShiftOperator() {
  {
    BigInt a(2);
    ++a;
    QCOMPARE(toBase2(a), "0000000000000001");

    BigInt b(2);
    b = a >> 1;
    QCOMPARE(toBase2(b), "0000000000000000");
  }

  {
    BigInt a(2);
    a.setValueAt(1, 0);
    QCOMPARE(toBase2(a), "0000000100000000");

    BigInt b(2);
    b = a >> 1;
    QCOMPARE(toBase2(b), "0000000010000000");
  }

  {
    BigInt a(2);
    a.setValueAt((uint8_t)0xFFFF, 0);
    QCOMPARE(toBase2(a), "1111111100000000");

    BigInt b(2);
    b = a >> 1;
    QCOMPARE(toBase2(b), "0111111110000000");
  }

  {
    BigInt a(16);
    for (int i = 0; i < 16; ++i) a.setValueAt((uint8_t)0xFFFF, i);
    QCOMPARE(toBase2(a),
             "11111111111111111111111111111111111111111111111111111111111111111"
             "111111111111111111111111111111111111111111111111111111111111111");

    BigInt b(16);
    b = a >> 87;
    QCOMPARE(toBase2(b),
             "00000000000000000000000000000000000000000000000000000000000000000"
             "000000000000000000000011111111111111111111111111111111111111111");
  }
}

static TestBigInt s_testBigInt;
