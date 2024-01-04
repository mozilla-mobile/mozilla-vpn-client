/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WASMINSPECTOR_H
#define WASMINSPECTOR_H

#include <QWebChannelAbstractTransport>

namespace InspectorServer {

class WasmInspector final : public QWebChannelAbstractTransport {
  Q_DISABLE_COPY_MOVE(WasmInspector)

 public:
  ~WasmInspector();

  // Consume a QByteArray of Data. 
  // Assumes the String to contain a json Object Produced
  // by a webchannel. 
  // Should only be called by emscripten Bindings. 
  void receive(const QByteArray& data);

 private:
  WasmInspector(QObject* parent);

  void sendMessage(const QJsonObject& message) override;


};
}  // namespace InspectorServer
#endif  // WASMINSPECTOR_H
