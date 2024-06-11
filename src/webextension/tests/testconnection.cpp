/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testconnection.h"

#include <QBuffer>
#include <QDataStream>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>

#include "connection.h"

QTEST_MAIN(TestConnection)

void TestConnection::testEmptyBuffer() {
  QEventLoop loop;
  QBuffer buffer;
  buffer.open(QIODevice::ReadWrite);
  WebExtension::Connection con(qApp, &buffer);
  loop.processEvents();
  // Nothing should have been written back.
  QCOMPARE(buffer.size(), 0);
}

void TestConnection::testZeroSized() {
  QEventLoop loop;
  QBuffer buffer;
  buffer.open(QIODevice::ReadWrite);
  WebExtension::Connection con(qApp, &buffer);
  //
  writeTo("{\"t\":\"valid\"}", 0, &buffer);
  buffer.seek(0);
  auto input_size = buffer.size();

  loop.processEvents();
  // Nothing should have been written back.
  QCOMPARE(buffer.size(), input_size);
  // The Connection should have been closed.
  QCOMPARE(buffer.isOpen(), false);
}

void TestConnection::testEmitsJSONMessages() {
  QEventLoop loop;
  QBuffer buffer;
  buffer.open(QIODevice::ReadWrite);
  WebExtension::Connection con(qApp, &buffer);

  // When it recieves valid json it should
  // emit that object raw.
  bool callbackFired = false;
  connect(&con, &WebExtension::Connection::onMessageReceived,
          [&](QJsonObject o) {
            callbackFired = true;
            QCOMPARE(o["t"].toString(), "valid");
          });

  writeTo("{\"t\":\"valid\"}", &buffer);
  buffer.seek(0);
  auto input_size = buffer.size();

  loop.processEvents();
  // It should have emitted something :)
  QCOMPARE(callbackFired, true);
}

void TestConnection::testInvalidJSONEmitsInvalid() {
  QList<QString> cases = {
      "a",              // not json
      "[\"a\",\"b\"]",  // arrays not allowed
  };

  for (QString testCase : cases) {
    QEventLoop loop;
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    WebExtension::Connection con(qApp, &buffer);

    // When it recieves valid json it should
    // emit that object raw.
    bool callbackFired = false;
    connect(&con, &WebExtension::Connection::onMessageReceived,
            [&](QJsonObject o) { callbackFired = true; });

    writeTo(testCase.toLocal8Bit(), &buffer);
    auto pos = buffer.pos();
    buffer.seek(0);
    loop.processEvents();

    // It should have emitted something :)
    QCOMPARE(callbackFired, false);
    auto objects = findObjects(buffer.buffer());
    QCOMPARE(objects.length(), 1);
    auto response = objects.last();
    QCOMPARE(response["t"].toString(), "invalidRequest");
  }
}

void TestConnection::writeTo(const QByteArray& data, QIODevice* target) {
  auto length = (uint32_t)data.length();
  writeTo(data, length, target);
}

void TestConnection::writeTo(const QByteArray& data, int len,
                             QIODevice* target) {
  char* rawLength = reinterpret_cast<char*>(&len);
  target->write(rawLength, sizeof(uint32_t));
  target->write(data.constData());
}

/*
 * Read's the whole buffer, returns all found json objects.
 * Non json messages are skipped.
 */
QList<QJsonObject> TestConnection::findObjects(const QByteArray data) {
  QList<QJsonObject> out;
  // The Format is easy
  int header_offset = 0;
  uint32_t body_len = 0;

  for (int header_offset = 0; header_offset < data.length();
       header_offset = (header_offset + 4) + body_len) {
    body_len = *(data.mid(header_offset, 4).constData());
    QByteArray body = data.mid(header_offset + 4, body_len);
    QJsonDocument json = QJsonDocument::fromJson(body);
    if (!json.isNull() && json.isObject()) {
      out.append(json.object());
    }
  }
  return out;
}
