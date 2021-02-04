/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WASMWINDOWCONTROLLER_H
#define WASMWINDOWCONTROLLER_H

#include <QMainWindow>
#include <QObject>

class WasmWindowController final : public QObject {
  Q_DISABLE_COPY_MOVE(WasmWindowController)

 public:
  WasmWindowController();
  ~WasmWindowController();

 private:
  void iconChanged(const QString& icon);

 private:
  QMainWindow m_window;
};

#endif  // WASMWINDOWCONTROLLER_H
