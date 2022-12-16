/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERHANDLER_H
#define SERVERHANDLER_H

#include <QTcpServer>

class ServerHandler final : public QTcpServer {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ServerHandler)

 public:
  ServerHandler();
  ~ServerHandler();

 private:
  void newConnectionReceived();
};

#endif  // SERVERHANDLER_H
