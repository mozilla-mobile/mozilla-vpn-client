/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WASMWINDOWCONTROLLER_H
#define WASMWINDOWCONTROLLER_H

#include <QMainWindow>
#include <QObject>

class MacOSMenuBar;
class QLabel;
class QMenuBar;

class WasmWindowController final : public QObject {
  Q_DISABLE_COPY_MOVE(WasmWindowController)

 public:
  WasmWindowController();
  ~WasmWindowController();

  static WasmWindowController* instance();

  void notification(const QString& title, const QString& message);

  void retranslate();

 private:
  void iconChanged();

 private:
  QMainWindow m_window;
  QLabel* m_notificationTitle = nullptr;
  QLabel* m_notificationMessage = nullptr;
  QMenuBar* m_systemTrayMenuBar = nullptr;
  MacOSMenuBar* m_macOSMenuBar = nullptr;
};

#endif  // WASMWINDOWCONTROLLER_H
