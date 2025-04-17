/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMMAND_H
#define COMMAND_H

#include "webextreader.h"

#include <QFileDevice>
#include <QObject>
#include <QThread>

class QByteArray;
class QJsonObject;
class WebExtReader;

class WebExtHandler final : public QObject {
  Q_OBJECT

 public:
  WebExtHandler(QFileDevice* d, QObject* parent = nullptr);
  ~WebExtHandler();

  // The methods we can handle locally are exposed as meta-methods.
  Q_INVOKABLE void bridge_ping(const QByteArray& msg);
  Q_INVOKABLE void proc_info(const QByteArray& msg);
  Q_INVOKABLE void start(const QByteArray& msg);

  void writeMsgStdout(const QByteArray& msg);
  void writeJsonStdout(const QJsonObject& msg);

 signals:
  void unhandledMessage(const QByteArray& msg);
  void eofReceived();

 private:
  void handleMessage(const QByteArray& msg);

  QThread m_thread;
  QFileDevice* m_output;
  WebExtReader* m_reader;
};

#endif  // COMMAND_H
