/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wasminspector.h"
#include "leakdetector.h"
#include "logger.h"

#include <QCoreApplication>

#include <emscripten/bind.h>
#include <emscripten/emscripten.h>

namespace {
Logger logger(LOG_INSPECTOR, "WasmInspector");

WasmInspector* s_inspector = nullptr;
}  // namespace

EMSCRIPTEN_KEEPALIVE void inspectorCommand(emscripten::val input) {
  std::string str = input.as<std::string>();
  WasmInspector::instance()->recv(QByteArray(str.c_str(), str.length()));
}

EMSCRIPTEN_BINDINGS(MozillaVPNIntegraton) {
  emscripten::function("inspectorCommand", &inspectorCommand);
}

EM_JS(void, call_inspectorMessage, (const char* msg),{ 
        try{
          inspectorMessage(JSON.parse(UTF8ToString(msg))); 
        }
        catch(e){
          console.log("Failed to deliver message from Inspector");
        }     
});

// static
WasmInspector* WasmInspector::instance() {
  if (!s_inspector) {
    s_inspector = new WasmInspector(qApp);
  }

  return s_inspector;
}

WasmInspector::WasmInspector(QObject* parent) : InspectorHandler(parent) {
  MVPN_COUNT_CTOR(WasmInspector);
}

WasmInspector::~WasmInspector() { MVPN_COUNT_DTOR(WasmInspector); }

void WasmInspector::send(const QByteArray& buffer) {
  call_inspectorMessage(buffer.constData());
}
