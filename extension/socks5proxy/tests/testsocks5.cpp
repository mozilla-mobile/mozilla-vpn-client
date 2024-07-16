/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testsocks5.h"

#include <QBuffer>
#include <QEventLoop>
#include <QFileInfo>
#include <QFuture>
#include <QNetworkProxy>
#include <QObject>
#include <QPromise>
#include <QRandomGenerator>
#include <QTcpServer>
#include <QTest>
#include <QTimer>

#include "socks5.h"
#include "socks5client.h"

constexpr auto testData = "Hello Moto";
#pragma region Helpers

/**
 * @brief Creates a Server on a given port
 * it will write ${data} onto the next TCP connection
 * then close  the connection and server.
 *
 * @param port
 * @param data
 * @return QFuture<bool> - A connection was established
 */
QFuture<bool> makeServer(uint port) {
  auto prom = std::make_unique<QPromise<bool>>();
  auto out = prom->future();
  auto server = new QTcpServer(qApp);
  prom->start();
  QObject::connect(server, &QTcpServer::newConnection,
                   [prom = std::move(prom), server]() {
                     qDebug() << "New pending connection!";
                     auto connection = server->nextPendingConnection();
                     connection->write(testData);
                     connection->flush();
                     // Dispatch onto the eventloop.
                     QTimer::singleShot(200, qApp, [connection, server]() {
                       connection->close();
                       connection->deleteLater();
                       server->close();
                       server->deleteLater();
                     });
                     prom->addResult(true);
                     prom->finish();
                   });
  qDebug() << "Server ready!!";
  server->listen(QHostAddress::LocalHost, port);
  return out;
};

uint16_t rollPort() {
  return static_cast<uint16_t>(
      QRandomGenerator::global()->bounded(49152, 65535));
};

QFuture<QByteArray> connectTo(uint serverPort, quint16 proxyPort) {
  auto prom = std::make_unique<QPromise<QByteArray>>();
  auto out = prom->future();
  auto socket = new QTcpSocket();
  socket->setProxy(QNetworkProxy{QNetworkProxy::ProxyType::Socks5Proxy,
                                 "localhost", proxyPort});
  prom->start();
  QObject::connect(socket, &QTcpSocket::readyRead,
                   [prom = std::move(prom), socket]() {
                     qDebug() << "Got something from server!";
                     prom->addResult(socket->readAll());
                     socket->close();
                     socket->deleteLater();
                     prom->finish();
                   });
  socket->connectToHost(QHostAddress::LocalHost, serverPort);
  return out;
};
#pragma endregion

/**
 * Create a TCP Server -  Sending "Hello Moto"
 * to the first incoming connection.
 *
 * Create a SocksProxy, and a TCP connection.
 * Setup the TCP Connection to connect to the proxy
 * Then dials the server and revices a string.
 *
 */
void TestSocks5::proxyTCP() {
  auto const proxyPort = rollPort();
  auto const serverPort = rollPort();
  auto const serverHadConnection = makeServer(serverPort);
  Socks5 proxy{nullptr, proxyPort, QHostAddress::LocalHost};
  auto const proxyHadConnection =
      QtFuture::connect(&proxy, &Socks5::incomingConnection);
  auto const proxyHadData =
      QtFuture::connect(&proxy, &Socks5::dataSentReceived);
  auto const connectionToServer = connectTo(serverPort, proxyPort);

  while (!connectionToServer.isFinished()) {
    QTest::qWait(250);
  };
  // The TCP Server should have gotten a connection
  QCOMPARE(serverHadConnection.result(), true);
  // Data Recieved should be 0
  QCOMPARE(std::get<0>(proxyHadData.result()), qsizetype(0));
  // Data Sent should be 10.
  QCOMPARE(std::get<1>(proxyHadData.result()), qsizetype(10));
  // The Proxy server should have gotten a connection
  QCOMPARE(QHostAddress{proxyHadConnection.result()}, QHostAddress::LocalHost);
  // We should have gotten the correct string
  QCOMPARE(connectionToServer.result(), QByteArray{testData});
}

QTEST_MAIN(TestSocks5)
