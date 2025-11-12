/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WEBEXTENSION_SERVER_H
#define WEBEXTENSION_SERVER_H

#include <QLocalServer>

#include "baseadapter.h"

namespace WebExtension {

class Server final : public QLocalServer {
  Q_OBJECT

 public:
  Server(BaseAdapter* adapter);
  ~Server();

  /**
   * @brief returns true if the socket is allowed to connect.
   */
  static bool isAllowedToConnect(qintptr sd);

 private:
  BaseAdapter* m_adapter;
  void newConnectionReceived();
};

}  // namespace WebExtension
#endif  // WEBEXTENSION_SERVER_H
