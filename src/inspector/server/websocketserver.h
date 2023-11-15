/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTORWEBSOCKETSERVER_H
#define INSPECTORWEBSOCKETSERVER_H

#include <QWebSocketServer>
class Inspector;
namespace InspectorServer {


class InspectorWebSocketServer final : public QWebSocketServer {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(InspectorWebSocketServer)

 public:
  explicit InspectorWebSocketServer(Inspector* parent);
  ~InspectorWebSocketServer();

 private:
  void newConnectionReceived();

  Inspector* m_parent = nullptr;
};

}  // namespace InspectorServer

#endif  // INSPECTORWEBSOCKETSERVER_H