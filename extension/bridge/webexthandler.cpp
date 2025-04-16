/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "webexthandler.h"

#include <QByteArray>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>

WebExtHandler::WebExtHandler(QFileDevice* d, QObject* parent) : QObject(parent) {
  m_output = d;
  m_stream.setByteOrder(QDataStream::LittleEndian);
  m_stream.setDevice(m_output);
}

void WebExtHandler::writeMsgStdout(const QByteArray& msg) {
  m_stream << static_cast<quint32>(msg.length());
  m_stream << msg;
  m_output->flush();
}

void WebExtHandler::writeJsonStdout(const QJsonObject& obj) {
  writeMsgStdout(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void WebExtHandler::writeStatus(const QString& status) {
  QJsonObject obj;
  obj["status"] = status;
  writeJsonStdout(obj);
}

void WebExtHandler::handleMessage(const QByteArray& msg) {
  QJsonDocument doc = QJsonDocument::fromJson(msg);
  if (!doc.isObject()) {
    qWarning() << "JSON command failed to parse";
    return;
  }
  QJsonObject obj = doc.object();
  if (!obj.contains("t")) {
    qWarning() << "JSON command missing type";
    return;
  }

  QString msgType = obj.value("t").toString();
  QByteArray signature = QString("%1(QByteArray)").arg(msgType).toLocal8Bit();
  int index = this->metaObject()->indexOfMethod(signature.constData());
  if (index >= 0) {
    // This command can be handled locally.
    QMetaObject::invokeMethod(this, msgType.toLocal8Bit().constData(), msg);
  } else {
    // Otherwise - we cannot handle this message.
    emit unhandledMessage(msg);
  }
}

void WebExtHandler::bridge_ping(const QByteArray& msg) {
  writeStatus("bridge_pong");
}
