/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "webexthandler.h"

#include <stdio.h>

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>

#include "webextreader.h"

WebExtHandler::WebExtHandler(QFileDevice* d, QObject* parent)
    : QObject(parent) {
  m_output = d;
  m_worker = new WebExtWorker(this);

  connect(m_worker, SIGNAL(messageReceived(QByteArray)), this,
          SLOT(handleMessage(QByteArray)));
  connect(m_worker, &WebExtWorker::eofReceived, this,
          [&]() { emit eofReceived(); });

  m_worker->start();
}

void WebExtHandler::writeMsgStdout(const QByteArray& msg) {
  quint32 length = msg.length();
  m_output->write(reinterpret_cast<const char*>(&length), sizeof(length));
  m_output->write(msg);
  m_output->flush();
}

void WebExtHandler::writeJsonStdout(const QJsonObject& obj) {
  writeMsgStdout(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void WebExtHandler::handleMessage(const QByteArray& msg) {
  // Parse the message type.
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(msg, &err);
  const QJsonValue msgType = doc["t"];
  if (!msgType.isString()) {
    emit unhandledMessage(msg);
    return;
  }

  QString name = msgType.toString();
  QByteArray signature = QString("%1(QByteArray)").arg(name).toLocal8Bit();
  int index = this->metaObject()->indexOfMethod(signature.constData());
  if (index >= 0) {
    // This command can be handled locally.
    QMetaObject::invokeMethod(this, name.toLocal8Bit().constData(),
                              Q_ARG(QByteArray, msg));
  } else {
    // Otherwise - we cannot handle this message.
    emit unhandledMessage(msg);
  }
}

void WebExtHandler::bridge_ping(const QByteArray& msg) {
  writeJsonStdout(QJsonObject({{"status", "bridge_pong"}}));
}

void WebExtWorker::run() {
  quint32 offset = 0;
  QByteArray buffer;

  while (!feof(stdin)) {
    // Read the message length and allocate the buffer.
    if (buffer.isEmpty()) {
      quint32 length;
      if (!fread(&length, sizeof(length), 1, stdin)) {
        continue;
      }
      buffer.resize(length);
      offset = 0;
    }

    // Read the message data into the buffer.
    quint32 maxsz = buffer.length() - offset;
    size_t rx = fread(buffer.data() + offset, 1, maxsz, stdin);
    Q_ASSERT(rx <= maxsz);
    offset += rx;
    if (offset < buffer.length()) {
      // We need more data.
      continue;
    }

    // Message has been received.
    emit messageReceived(buffer);
    buffer.clear();
    offset = 0;
  }

  emit eofReceived();
}
