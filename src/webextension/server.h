/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WEBEXTENSION_SERVER_H
#define WEBEXTENSION_SERVER_H

#include <QTcpServer>

#include "baseadapter.h"

namespace WebExtension {

class Server final : public QTcpServer {
  Q_OBJECT

 public:
  Server(BaseAdapter* adapter);
  ~Server();

  // Checks if this socket may connect to the client.
  static bool isAllowedToConnect(QHostAddress sock);

 private:
  BaseAdapter* m_adapter;
  void newConnectionReceived();
};

}  // namespace WebExtension
#endif  // WEBEXTENSION_SERVER_H
