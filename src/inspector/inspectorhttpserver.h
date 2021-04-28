/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTORHTTPSERVER_H
#define INSPECTORHTTPSERVER_H

#include <QTcpServer>

class InspectorHttpServer final : public QTcpServer {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(InspectorHttpServer)

 public:
  explicit InspectorHttpServer(QObject* parent);
  ~InspectorHttpServer();

 private:
  void newConnectionReceived();
};

#endif  // INSPECTORHTTPSERVER_H
