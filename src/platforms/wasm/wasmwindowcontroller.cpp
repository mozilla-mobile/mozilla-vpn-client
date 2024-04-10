/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wasmwindowcontroller.h"

#include <QGuiApplication>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QScreen>
#include <QVBoxLayout>
#include <QWindow>

#include "logger.h"
#include "mozillavpn.h"
#include "notificationhandler.h"
#include "platforms/macos/macosmenubar.h"
#include "statusicon.h"
#include "utils/leakdetector/leakdetector.h"

namespace {
Logger logger("WasmWindowController");

WasmWindowController* s_instance = nullptr;
}  // namespace

WasmWindowController::WasmWindowController() {
  MZ_COUNT_CTOR(WasmWindowController);
  Q_ASSERT(s_instance == nullptr);
  s_instance = this;

  QList<QScreen*> screens = qApp->screens();
  if (screens.length() < 2) {
    logger.debug() << "Only 1 screen detected. No menu for wasm";
    return;
  }

  logger.debug() << "Wasm control window creation";

  QWidget* centralWidget = new QWidget(&m_window);
  m_window.setCentralWidget(centralWidget);

  QVBoxLayout* layout = new QVBoxLayout(centralWidget);

  // System tray icon
  {
    QLabel* label = new QLabel("System tray menu:");
    layout->addWidget(label);

    m_systemTrayMenuBar = new QMenuBar();
    layout->addWidget(m_systemTrayMenuBar);

    StatusIcon* statusIcon = MozillaVPN::instance()->statusIcon();
    connect(statusIcon, &StatusIcon::iconUpdateNeeded, this,
            &WasmWindowController::iconChanged);
    iconChanged();

    QMenu* menu = NotificationHandler::instance()->contextMenu();
    m_systemTrayMenuBar->addMenu(menu);
  }

  // MacOS Menu bar
  {
    QLabel* label = new QLabel("MacOS menu:");
    layout->addWidget(label);

    m_macOSMenuBar = new MacOSMenuBar();
    m_macOSMenuBar->initialize();
    layout->addWidget(m_macOSMenuBar->menuBar());
  }

  // Notification title
  {
    QLabel* label = new QLabel("Last notification Title:");
    layout->addWidget(label);

    m_notificationTitle = new QLabel();
    m_notificationTitle->setStyleSheet("font-weight: bold");
    layout->addWidget(m_notificationTitle);
  }

  // Notification message
  {
    QLabel* label = new QLabel("Last notification Message:");
    layout->addWidget(label);

    m_notificationMessage = new QLabel();
    m_notificationMessage->setStyleSheet("font-weight: bold");
    layout->addWidget(m_notificationMessage);
  }

  // stratch
  layout->addWidget(new QWidget(), 1);

  m_window.showFullScreen();

  // System tray has a different message for internal notifications (not
  // related to the VPN status).
  connect(NotificationHandler::instance(),
          &NotificationHandler::notificationShown, this,
          &WasmWindowController::notification);
}

WasmWindowController::~WasmWindowController() {
  MZ_COUNT_DTOR(WasmWindowController);
  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

// static
WasmWindowController* WasmWindowController::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

void WasmWindowController::iconChanged() {
  StatusIcon* statusIcon = MozillaVPN::instance()->statusIcon();

  QMenu* menu = NotificationHandler::instance()->contextMenu();
  menu->setIcon(statusIcon->icon());
}

void WasmWindowController::notification(const QString& title,
                                        const QString& message) {
  logger.debug() << "Notification received";

  m_notificationTitle->setText(title);
  m_notificationMessage->setText(message);
}

void WasmWindowController::retranslate() {
  QMenu* menu = NotificationHandler::instance()->contextMenu();
  m_systemTrayMenuBar->addMenu(menu);

  m_macOSMenuBar->retranslate();
}
