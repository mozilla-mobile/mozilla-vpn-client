/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testSocks5client.h"

#include <QBuffer>
#include <QEventLoop>
#include <QObject>
#include <QTest>

#include "socks5client.h"

/**
 * Data should be copied, in order from rx to tx
 */
void TestSocks5Client::proxy() {
  QBuffer rxBuffer;
  QBuffer txBuffer;
  rxBuffer.open(QIODevice::ReadWrite);
  rxBuffer.write("abc");
  rxBuffer.seek(0);
  txBuffer.open(QIODevice::ReadWrite);
  const auto bytesWritten = Socks5Client::proxy(&rxBuffer, &txBuffer);
  QCOMPARE(bytesWritten, 3);

  txBuffer.seek(0);
  const auto output = txBuffer.readAll();
  QCOMPARE(output, "abc");
}

/**
 * An empty device should just not
 * forward any device
 */
void TestSocks5Client::proxyEmpty() {
  QBuffer rxBuffer;
  QBuffer txBuffer;
  rxBuffer.open(QIODevice::ReadWrite);
  txBuffer.open(QIODevice::ReadWrite);
  const auto bytesWritten = Socks5Client::proxy(&rxBuffer, &txBuffer);
  QCOMPARE(bytesWritten, 0);
}

/**
 * In case of writing to a closed
 * IODevice it should return an error (-1)
 */
void TestSocks5Client::proxyClosed() {
  QBuffer rxBuffer;
  QBuffer txBuffer;

  QByteArray data = {10, 'x'};
  rxBuffer.open(QIODevice::ReadWrite);
  rxBuffer.write(data);
  rxBuffer.seek(0);

  const auto bytesWritten = Socks5Client::proxy(&rxBuffer, &txBuffer);
  QCOMPARE(bytesWritten, -1);
}

QTEST_MAIN(TestSocks5Client)
