/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTORSERVER_H
#define INSPECTORSERVER_H

#include <QWebSocketServer>

class InspectorServer final : public QWebSocketServer {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(InspectorServer)

 public:
  InspectorServer();
  ~InspectorServer();

 private:
  void newConnectionReceived();
};

#endif  // INSPECTORSERVER_H
