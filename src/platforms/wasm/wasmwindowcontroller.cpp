/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wasmwindowcontroller.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "platforms/macos/macosmenubar.h"
#include "systemtrayhandler.h"

#include <QGuiApplication>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QScreen>
#include <QVBoxLayout>
#include <QWindow>

namespace {
Logger logger(LOG_MAIN, "WasWindowController");
}

WasmWindowController::WasmWindowController() {
  MVPN_COUNT_CTOR(WasmWindowController);

  QList<QScreen*> screens = qApp->screens();
  if (screens.length() < 2) {
    logger.log() << "Only 1 screen detected. No menu for wasm";
    return;
  }

  logger.log() << "Wasm control window creation";

  QWidget* centralWidget = new QWidget(&m_window);
  m_window.setCentralWidget(centralWidget);

  QVBoxLayout* layout = new QVBoxLayout(centralWidget);

  // System tray icon
  {
    QLabel* label = new QLabel("System tray menu:");
    layout->addWidget(label);

    QMenuBar* menuBar = new QMenuBar();
    layout->addWidget(menuBar);

    StatusIcon* statusIcon = MozillaVPN::instance()->statusIcon();
    connect(statusIcon, &StatusIcon::iconChanged, this,
            &WasmWindowController::iconChanged);
    iconChanged(statusIcon->iconString());

    QMenu* menu = SystemTrayHandler::instance()->contextMenu();
    menuBar->addMenu(menu);
  }

  // MacOS Menu bar
  {
    QLabel* label = new QLabel("MacOS menu:");
    layout->addWidget(label);

    MacOSMenuBar* menu = new MacOSMenuBar();
    menu->initialize();
    layout->addWidget(menu->menuBar());
  }

  // stratch
  layout->addWidget(new QWidget(), 1);

  m_window.show();
}

WasmWindowController::~WasmWindowController() {
  MVPN_COUNT_DTOR(WasmWindowController);
}

void WasmWindowController::iconChanged(const QString& icon) {
  QIcon menuIcon(icon);
  menuIcon.setIsMask(true);

  QMenu* menu = SystemTrayHandler::instance()->contextMenu();
  menu->setIcon(menuIcon);
}
