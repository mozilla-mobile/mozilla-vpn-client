/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testhkdf.h"

#include <QByteArray>
#include <QGlobalStatic>
#include <QString>
#include <QtTest/QtTest>

#include "hkdf.h"

void TestHKDF::rfc5869_data() {
  QTest::addColumn<QCryptographicHash::Algorithm>("algorithm");
  QTest::addColumn<QByteArray>("ikm");
  QTest::addColumn<QByteArray>("salt");
  QTest::addColumn<QByteArray>("info");
  QTest::addColumn<QByteArray>("okm");

  QTest::addRow("Basic test case with SHA-256")
      << QCryptographicHash::Sha256
      << fromHex("0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b",
                 "0b0b0b0b0b0b")
      << fromHex("000102030405060708090a0b0c")
      << fromHex("f0f1f2f3f4f5f6f7f8f9")
      << fromHex("3cb25f25faacd57a90434f64d0362f2a"
                 "2d2d0a90cf1a5a4c5db02d56ecc4c5bf",
                 "34007208d5b887185865");

  QTest::addRow("Test with SHA-256 and longer inputs/outputs")
      << QCryptographicHash::Sha256
      << fromHex("000102030405060708090a0b0c0d0e0f",
                 "101112131415161718191a1b1c1d1e1f",
                 "202122232425262728292a2b2c2d2e2f",
                 "303132333435363738393a3b3c3d3e3f",
                 "404142434445464748494a4b4c4d4e4f")
      << fromHex("606162636465666768696a6b6c6d6e6f",
                 "707172737475767778797a7b7c7d7e7f",
                 "808182838485868788898a8b8c8d8e8f",
                 "909192939495969798999a9b9c9d9e9f",
                 "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf")
      << fromHex("b0b1b2b3b4b5b6b7b8b9babbbcbdbebf",
                 "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf",
                 "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf",
                 "e0e1e2e3e4e5e6e7e8e9eaebecedeeef",
                 "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff")
      << fromHex("b11e398dc80327a1c8e7f78c596a4934",
                 "4f012eda2d4efad8a050cc4c19afa97c",
                 "59045a99cac7827271cb41c65e590e09",
                 "da3275600c2f09b8367793a9aca3db71",
                 "cc30c58179ec3e87c14c01d5c1f3434f",
                 "1d87");

  QTest::addRow("Test with SHA-256 and zero-length salt/info")
      << QCryptographicHash::Sha256
      << fromHex("0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b")
      << QByteArray()
      << QByteArray()
      << fromHex("8da4e775a563c18f715f802a063c5a31",
                 "b8a11f5c5ee1879ec3454e5f3c738d2d",
                 "9d201395faa4b61a96c8");

  QTest::addRow("Basic test case with SHA-1")
      << QCryptographicHash::Sha1
      << fromHex("0b0b0b0b0b0b0b0b0b0b0b")
      << fromHex("000102030405060708090a0b0c")
      << fromHex("f0f1f2f3f4f5f6f7f8f9")
      << fromHex("085a01ea1b10f36933068b56efa5ad81",
                 "a4f14b822f5b091568a9cdd4f155fda2",
                 "c22e422478d305f3f896");

  QTest::addRow("Test with SHA-1 and longer inputs/outputs")
      << QCryptographicHash::Sha1
      << fromHex("000102030405060708090a0b0c0d0e0f",
                 "101112131415161718191a1b1c1d1e1f",
                 "202122232425262728292a2b2c2d2e2f",
                 "303132333435363738393a3b3c3d3e3f",
                 "404142434445464748494a4b4c4d4e4f")
      << fromHex("606162636465666768696a6b6c6d6e6f",
                 "707172737475767778797a7b7c7d7e7f",
                 "808182838485868788898a8b8c8d8e8f",
                 "909192939495969798999a9b9c9d9e9f",
                 "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf")
      << fromHex("b0b1b2b3b4b5b6b7b8b9babbbcbdbebf",
                 "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf",
                 "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf",
                 "e0e1e2e3e4e5e6e7e8e9eaebecedeeef",
                 "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff")
      << fromHex("0bd770a74d1160f7c9f12cd5912a06eb",
                 "ff6adcae899d92191fe4305673ba2ffe",
                 "8fa3f1a4e5ad79f3f334b3b202b2173c",
                 "486ea37ce3d397ed034c7f9dfeb15c5e",
                 "927336d0441f4c4300e2cff0d0900b52",
                 "d3b4");

  QTest::addRow("Test with SHA-1 and zero-length salt/info")
      << QCryptographicHash::Sha1
      << fromHex("0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b")
      << QByteArray()
      << QByteArray()
      << fromHex("0ac1af7002b3d761d1e55298da9d0506"
                 "b9ae52057220a306e07b6b87e8df21d0"
                 "ea00033de03984d34918");
}

void TestHKDF::rfc5869() {
  QFETCH(QCryptographicHash::Algorithm, algorithm);
  QFETCH(QByteArray, ikm);
  QFETCH(QByteArray, salt);
  QFETCH(QByteArray, info);
  QFETCH(QByteArray, okm);

  HKDF hkdf(algorithm, salt);
  hkdf.addData(ikm);
  QCOMPARE(hkdf.result(okm.length(), info), okm);
}
