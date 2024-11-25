/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testsocks5connection.h"

#include <QBuffer>
#include <QEventLoop>
#include <QObject>
#include <QTcpServer>
#include <QTest>

#include "socks5connection.h"

/**
 * Data should be copied, in order from rx to tx
 */
void TestSocks5Connection::proxy() {
  QBuffer rxBuffer;
  QBuffer txBuffer;
  quint64 watermark = 0;
  rxBuffer.open(QIODevice::ReadWrite);
  rxBuffer.write("abc");
  rxBuffer.seek(0);
  txBuffer.open(QIODevice::ReadWrite);
  Socks5Connection::proxy(&rxBuffer, &txBuffer, watermark);
  QCOMPARE(txBuffer.buffer().size(), 3);

  txBuffer.seek(0);
  const auto output = txBuffer.readAll();
  QCOMPARE(output, "abc");
}

/**
 * An empty device should just not
 * forward any device
 */
void TestSocks5Connection::proxyEmpty() {
  QBuffer rxBuffer;
  QBuffer txBuffer;
  quint64 watermark = 0;
  rxBuffer.open(QIODevice::ReadWrite);
  txBuffer.open(QIODevice::ReadWrite);
  Socks5Connection::proxy(&rxBuffer, &txBuffer, watermark);
  QCOMPARE(txBuffer.buffer().size(), 0);
}

/**
 * No data should be written to a closed device.
 */
void TestSocks5Connection::proxyClosed() {
  QBuffer rxBuffer;
  QBuffer txBuffer;
  quint64 watermark = 0;

  QByteArray data = {10, 'x'};
  rxBuffer.open(QIODevice::ReadWrite);
  rxBuffer.write(data);
  rxBuffer.seek(0);

  Socks5Connection::proxy(&rxBuffer, &txBuffer, watermark);
  QCOMPARE(txBuffer.buffer().size(), 0);
}

/**
 * Writes should be flow controlled so that `bytesToWrite()` never exceeds
 * 16kB
 */
void TestSocks5Connection::proxyFlowControl() {
  constexpr const int FIRST_FRAGMENT_SIZE = 1234;
  constexpr const int PROXY_MAX_BUFFER_SIZE = 16 * 1024;

  // We'll need to use a TCP to test out flow control. So create a TCP server.
  QTcpServer server;
  QCOMPARE(server.listen(QHostAddress::LocalHost), true);

  // Create a TCP socket and connect to the server.
  QTcpSocket sendSocket;
  sendSocket.connectToHost(QHostAddress::LocalHost, server.serverPort());

  // There should be a socket we can now accept.
  QCOMPARE(server.waitForNewConnection(500), true);
  QTcpSocket* recvSocket = server.nextPendingConnection();
  recvSocket->setParent(&server);
  QVERIFY(recvSocket != nullptr);

  // The dummy socket should now be connected too.
  QCOMPARE(sendSocket.waitForConnected(500), true);
  QCOMPARE(sendSocket.state(), QAbstractSocket::ConnectedState);

  // Prepare a small fragment of data to send through the socket.
  QByteArray firstData(FIRST_FRAGMENT_SIZE, 'Q');
  QBuffer firstBuffer(&firstData);
  firstBuffer.open(QIODevice::ReadOnly);

  // Send the first fragment of data, and check the high watermark.
  quint64 watermark = 0;
  Socks5Connection::proxy(&firstBuffer, &sendSocket, watermark);
  QCOMPARE(watermark, FIRST_FRAGMENT_SIZE);
  QCOMPARE(firstBuffer.pos(), FIRST_FRAGMENT_SIZE);
  QCOMPARE(sendSocket.bytesToWrite(), FIRST_FRAGMENT_SIZE);

  // Send more data to try and overflow the outbound socket buffer.
  QByteArray hugeData(PROXY_MAX_BUFFER_SIZE, 'Z');
  QBuffer hugeBuffer(&hugeData);
  hugeBuffer.open(QIODevice::ReadOnly);
  Socks5Connection::proxy(&hugeBuffer, &sendSocket, watermark);
  QCOMPARE(watermark, PROXY_MAX_BUFFER_SIZE);
  QCOMPARE(hugeBuffer.pos(), PROXY_MAX_BUFFER_SIZE - FIRST_FRAGMENT_SIZE);
  QCOMPARE(sendSocket.bytesToWrite(), PROXY_MAX_BUFFER_SIZE);

  // Trying to send again should have no effect.
  Socks5Connection::proxy(&hugeBuffer, &sendSocket, watermark);
  QCOMPARE(watermark, PROXY_MAX_BUFFER_SIZE);
  QCOMPARE(hugeBuffer.pos(), PROXY_MAX_BUFFER_SIZE - FIRST_FRAGMENT_SIZE);
  QCOMPARE(sendSocket.bytesToWrite(), PROXY_MAX_BUFFER_SIZE);

  // Send the data on the socket.
  while (sendSocket.bytesToWrite() > 0) {
    QCOMPARE(sendSocket.waitForBytesWritten(500), true);
  }

  // On the receiving end of the socket we should be able to read one
  // buffer's worth of data.
  QByteArray result;
  while(recvSocket->waitForReadyRead(500)) {
    result.append(recvSocket->readAll());
  }
  QCOMPARE(result.length(), PROXY_MAX_BUFFER_SIZE);

  // And we should now be able to send the last fragment of unwritten data.
  watermark = 0;
  qint64 expect = hugeData.length() - hugeBuffer.pos();
  Socks5Connection::proxy(&hugeBuffer, &sendSocket, watermark);
  QCOMPARE(watermark, expect);
  QCOMPARE(hugeBuffer.pos(), hugeData.length());
  QCOMPARE(sendSocket.bytesToWrite(), expect);
}

QTEST_MAIN(TestSocks5Connection)
