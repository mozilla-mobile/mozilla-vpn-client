/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wasminspector.h"

#include <emscripten/bind.h>
#include <emscripten/emscripten.h>

#include <QCoreApplication>
#include <QJsonDocument>

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("WasmInspector");

WasmInspector* s_inspector = nullptr;
}  // namespace


namespace InspectorServer {

EMSCRIPTEN_KEEPALIVE void inspectorCommand(emscripten::val input) {
  std::string str = input.as<std::string>();
  s_inspector->recv(QByteArray(str.c_str(), str.length()));
}

EMSCRIPTEN_BINDINGS(MozillaVPNIntegraton) {
  emscripten::function("inspectorCommand", &inspectorCommand);
}

EM_JS(void, call_inspectorMessage, (const char* msg), {
  try {
    inspectorMessage(JSON.parse(UTF8ToString(msg)));
  } catch (e) {
    console.log("Failed to deliver message from Inspector");
  }
});

void InspectorServer::WasmInspector::receive(const QByteArray& data) {
   QJsonParseError jsonError;
   QJsonDocument json = QJsonDocument::fromJson(message, &jsonError);
   if (QJsonParseError::NoError != jsonError.error) {
     return;
   }
   if (!json.isObject()) {
      return;
   }
   emit messageReceived(json.object(), this);
}

WasmInspector::WasmInspector(QObject* parent)
    : QWebChannelAbstractTransport(parent) {
  MZ_COUNT_CTOR(WasmInspector);
  if (!s_inspector) {
    s_inspector = new WasmInspector(qApp);
  }
}

WasmInspector::~WasmInspector() { MZ_COUNT_DTOR(WasmInspector); }

void WasmInspector::sendMessage(const QJsonObject& message) {
  QJsonDocument json;
  json.setObject(message);
  auto json_bytes = json.toJson(QJsonDocument::JsonFormat::Compact);
  call_inspectorMessage(json_bytes.constData());
}

}  // namespace InspectorServer