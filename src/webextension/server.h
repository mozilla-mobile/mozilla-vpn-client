/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERHANDLER_H
#define SERVERHANDLER_H

#include <QTcpServer>

#include "baseadapter.h"

namespace WebExtension {

class Server final : public QTcpServer {
  Q_OBJECT

 public:
  Server(BaseAdapter* adapter);
  ~Server();

  // Checks if this socket May connect to the Client.
  static bool isAllowedToConnect(QHostAddress sock);

 private:
  BaseAdapter* m_adapter;
  void newConnectionReceived();
};

}  // namespace WebExtension
#endif  // SERVERHANDLER_H
