/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WASMINSPECTOR_H
#define WASMINSPECTOR_H

#include "inspector/inspectorhandler.h"

class WasmInspector final : public InspectorHandler {
  Q_DISABLE_COPY_MOVE(WasmInspector)

 public:
  static WasmInspector* instance();
  ~WasmInspector();

 private:
  WasmInspector(QObject* parent);

  void send(const QByteArray& buffer) override;
};

#endif  // WASMINSPECTOR_H
