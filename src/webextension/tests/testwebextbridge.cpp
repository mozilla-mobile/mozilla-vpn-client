/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testwebextbridge.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QRandomGenerator>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <memory>

#include "webextbridge.h"

QTEST_MAIN(TestWebExtBridge)

// Returns a unique socket name for each test run to avoid conflicts.
static QString uniqueSocketName() {
  return QStringLiteral("mzvpn-test-%1")
      .arg(QRandomGenerator::global()->generate(), 8, 16, QLatin1Char('0'));
}

// ---------------------------------------------------------------------------
// EchoConnection: echoes all bytes back, byte-by-byte when fuzzyMs > 0.
// ---------------------------------------------------------------------------
class EchoConnection : public QObject {
  Q_OBJECT

 public:
  EchoConnection(QLocalSocket* socket, int fuzzyMs, QObject* parent = nullptr)
      : QObject(parent), m_socket(socket), m_fuzzyMs(fuzzyMs) {
    m_timer.setSingleShot(true);

    connect(m_socket, &QLocalSocket::readyRead, this,
            &EchoConnection::onReadyRead);
    if (fuzzyMs > 0) {
      connect(&m_timer, &QTimer::timeout, this, &EchoConnection::onTimer);
    }
  }

 private slots:
  void onReadyRead() {
    QByteArray data = m_socket->readAll();
    if (!m_fuzzyMs) {
      m_socket->write(data);
      m_socket->flush();
    } else {
      m_buf.append(data);
      if (!m_timer.isActive()) {
        m_timer.start(m_fuzzyMs);
      }
    }
  }

  void onTimer() {
    if (!m_buf.isEmpty()) {
      m_socket->write(m_buf.constData(), 1);
      m_socket->flush();
      m_buf.remove(0, 1);
      if (!m_buf.isEmpty()) {
        m_timer.start(m_fuzzyMs);
      }
    }
  }

 private:
  QLocalSocket* m_socket;
  QByteArray m_buf;
  QTimer m_timer;
  int m_fuzzyMs;
};

// ---------------------------------------------------------------------------
// EchoServer: accepts connections and creates an EchoConnection per client.
// ---------------------------------------------------------------------------
class EchoServer : public QLocalServer {
  Q_OBJECT

 public:
  explicit EchoServer(int fuzzyMs = 0, QObject* parent = nullptr)
      : QLocalServer(parent), m_fuzzyMs(fuzzyMs) {
    connect(this, &QLocalServer::newConnection, this,
            &EchoServer::onNewConnection);
  }

 private slots:
  void onNewConnection() {
    QLocalSocket* client = nextPendingConnection();
    // EchoConnection is owned by the client socket.
    new EchoConnection(client, m_fuzzyMs, client);
  }

 private:
  int m_fuzzyMs;
};

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

void TestWebExtBridge::app_ping_failure() {
  // A bridge pointing at a non-existent socket should reject sendMessage().
  WebExtBridge bridge(uniqueSocketName());
  QVERIFY(!bridge.sendMessage(QByteArray("\"ping\"")));
}

void TestWebExtBridge::app_ping_success() {
  const QString name = uniqueSocketName();

  EchoServer server;
  QVERIFY(server.listen(name));

  WebExtBridge bridge(name);

  // Wait for the bridge to connect (the bridge retries after a short delay).
  QSignalSpy connectedSpy(&bridge, &WebExtBridge::connected);
  QVERIFY(connectedSpy.wait(5000));

  // Send three messages and verify each is echoed back.
  QSignalSpy msgSpy(&bridge, &WebExtBridge::messageReceived);
  for (int i = 0; i < 3; ++i) {
    const QByteArray msg = QStringLiteral("\"hello world %1\"").arg(i).toUtf8();
    QVERIFY(bridge.sendMessage(msg));
    QVERIFY(msgSpy.wait(5000));
    QCOMPARE(msgSpy.last().at(0).toByteArray(), msg);
  }
}

void TestWebExtBridge::async_connection() {
  const QString name = uniqueSocketName();

  // Bridge created before server exists — send must fail immediately.
  WebExtBridge bridge(name);
  QVERIFY(!bridge.sendMessage(QByteArray("\"async connection\"")));

  // Now start the server.
  EchoServer server;
  QVERIFY(server.listen(name));

  // Wait for the bridge's retry timer to reconnect.
  QSignalSpy connectedSpy(&bridge, &WebExtBridge::connected);
  QVERIFY(connectedSpy.wait(5000));

  // Messages should now flow.
  QSignalSpy msgSpy(&bridge, &WebExtBridge::messageReceived);
  const QByteArray msg = "\"async connection\"";
  QVERIFY(bridge.sendMessage(msg));
  QVERIFY(msgSpy.wait(5000));
  QCOMPARE(msgSpy.last().at(0).toByteArray(), msg);
}

void TestWebExtBridge::async_disconnection() {
  const QString name = uniqueSocketName();

  // Use a heap-allocated server so we can explicitly destroy it (and its
  // accepted child sockets) to trigger disconnection on the bridge side.
  // server.close() only stops listening; it does NOT close accepted sockets.
  auto server = std::make_unique<EchoServer>();
  QVERIFY(server->listen(name));

  WebExtBridge bridge(name);

  QSignalSpy connectedSpy(&bridge, &WebExtBridge::connected);
  QVERIFY(connectedSpy.wait(5000));

  // Verify communication works while connected.
  QSignalSpy msgSpy(&bridge, &WebExtBridge::messageReceived);
  const QByteArray msg = "\"async disconnection\"";
  QVERIFY(bridge.sendMessage(msg));
  QVERIFY(msgSpy.wait(5000));
  QCOMPARE(msgSpy.last().at(0).toByteArray(), msg);

  // Destroy the server — this closes its accepted child sockets, which drops
  // the established connection and triggers bridge's disconnected signal.
  QSignalSpy disconnectedSpy(&bridge, &WebExtBridge::disconnected);
  server.reset();
  QVERIFY(disconnectedSpy.wait(5000));

  // Further sends should fail.
  QVERIFY(!bridge.sendMessage(msg));
}

void TestWebExtBridge::fuzzy() {
  for (int fuzzyMs : {1, 10, 100}) {
    const QString name = uniqueSocketName();

    EchoServer server(fuzzyMs);
    QVERIFY(server.listen(name));

    WebExtBridge bridge(name);

    QSignalSpy connectedSpy(&bridge, &WebExtBridge::connected);
    QVERIFY(connectedSpy.wait(5000));

    QSignalSpy msgSpy(&bridge, &WebExtBridge::messageReceived);
    for (int i = 0; i < 3; ++i) {
      const QByteArray msg =
          QStringLiteral("\"fuzzy %1 msg %2\"").arg(fuzzyMs).arg(i).toUtf8();
      QVERIFY(bridge.sendMessage(msg));
      // Generous timeout since the echo is intentionally slow (byte-by-byte).
      QVERIFY(msgSpy.wait(10000));
      QCOMPARE(msgSpy.last().at(0).toByteArray(), msg);
    }
  }
}

#include "testwebextbridge.moc"
