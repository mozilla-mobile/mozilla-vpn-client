/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testcurve25519.h"

#include <QByteArray>
#include <QGlobalStatic>
#include <QString>
#include <QtTest/QtTest>

#include "curve25519.h"

void TestCurve25519::rfc7748vectors_data() {
  QTest::addColumn<QString>("privkey");
  QTest::addColumn<QString>("pubkey");

  QTest::addRow("alice")
      << "77076d0a7318a57d3c16c17251b26645df4c2f87ebc0992ab177fba51db92c2a"
      << "8520f0098930a754748b7ddcb43ef75a0dbf3a0d26381af4eba4a98eaa9b4e6a";

  QTest::addRow("bob")
      << "5dab087e624a8a4b79e17f8b83800ee66f3bb1292618b6fd1c2f8b27ff88e0eb"
      << "de9edb7d7b7dc1b4d35b61c2ece435373f8343c85b78674dadfc7e146f882b4f";
}

void TestCurve25519::rfc7748vectors() {
  QFETCH(QString, privkey);
  QFETCH(QString, pubkey);

  QByteArray inputBase64 = QByteArray::fromHex(privkey.toUtf8()).toBase64();
  QByteArray expectBase64 = QByteArray::fromHex(pubkey.toUtf8()).toBase64();
  QByteArray result = Curve25519::generatePublicKey(inputBase64);
  QCOMPARE(result, expectBase64);
}
