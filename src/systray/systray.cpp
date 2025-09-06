/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systray.h"

#include <QApplication>
#include <QIcon>
#include <QQmlComponent>
#include <QQmlContext>
#include <QScreen>
#include <QSystemTrayIcon>

#include "qmlengineholder.h"
#include "utils/leakdetector.h"
#include "utils/logger.h"

namespace {
Logger logger("SysTray");
}

SysTray::SysTray(QObject* parent) : QObject(parent) { MZ_COUNT_CTOR(SysTray); }

SysTray::~SysTray() {
  MZ_COUNT_DTOR(SysTray);

  if (m_window) {
    m_window->deleteLater();
  }
  if (m_trayIcon) {
    m_trayIcon->deleteLater();
  }
  if (m_trayMenu) {
    m_trayMenu->deleteLater();
  }
}

void SysTray::initialize() {
  logger.debug() << "Initializing SysTray";

  if (!QSystemTrayIcon::isSystemTrayAvailable()) {
    logger.error() << "System tray is not available";
    return;
  }

  createTrayIcon();
  createContextMenu();
  setupWindow();
}

void SysTray::createTrayIcon() {
  logger.debug() << "Creating tray icon";

  m_trayIcon = new QSystemTrayIcon(this);

  // Use a simple icon for now
  QIcon icon(":/ui/resources/logo.svg");
  m_trayIcon->setIcon(icon);
  m_trayIcon->setToolTip("Mozilla VPN SysTray");

  connect(m_trayIcon, &QSystemTrayIcon::activated, this,
          &SysTray::onTrayIconActivated);

  m_trayIcon->show();
}

void SysTray::createContextMenu() {
  logger.debug() << "Creating context menu";

  m_trayMenu = new QMenu();

  QAction* showAction = m_trayMenu->addAction("Show/Hide");
  connect(showAction, &QAction::triggered, this, &SysTray::showWindow);

  QAction* quitAction = m_trayMenu->addAction("Quit");
  connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

  m_trayIcon->setContextMenu(m_trayMenu);
}

void SysTray::setupWindow() {
  logger.debug() << "Setting up QML window";

  auto engine = QmlEngineHolder::instance()->engine();

  // Load the QML component from the qt_add_qml_module generated path
  QQmlComponent component(engine, QUrl("qrc:/qt/qml/SysTray/SysTray.qml"));

  if (component.status() == QQmlComponent::Error) {
    logger.error() << "Failed to load QML component:";
    for (const QQmlError& error : component.errors()) {
      logger.error() << "  " << error.toString();
    }
    return;
  }

  if (component.status() != QQmlComponent::Ready) {
    logger.error() << "QML component is not ready, status:"
                   << component.status();
    return;
  }

  // Create the window object from the component
  QObject* object = component.create();
  if (!object) {
    logger.error() << "Failed to create object from QML component";
    return;
  }

  m_window = qobject_cast<QQuickWindow*>(object);
  if (!m_window) {
    logger.error() << "Failed to cast to QQuickWindow - object type:"
                   << object->metaObject()->className();
    object->deleteLater();
    return;
  }
  m_window->setFlags(Qt::Popup | Qt::FramelessWindowHint);

  logger.debug() << "QQuickWindow created successfully";

  // Hide initially
  m_window->hide();
  logger.debug() << "Window setup complete";
}

void SysTray::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
  logger.debug() << "Tray icon activated";

  switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
      if (m_window) {
        if (m_window->isVisible()) {
          hideWindow();
        } else {
          showWindow();
        }
      } else {
        setupWindow();
        showWindow();
      }
      break;
    default:
      break;
  }
}

void SysTray::showWindow() {
  logger.debug() << "Showing window";

  if (!m_window) {
    logger.error() << "Cannot show window - m_window is null";
    return;
  }

  logger.debug() << "Window is valid, proceeding to show";

  // Position the window near the system tray
  if (m_trayIcon && m_trayIcon->geometry().isValid()) {
    QRect trayGeometry = m_trayIcon->geometry();
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();

    logger.debug() << "Tray geometry: x=" << trayGeometry.x()
                   << " y=" << trayGeometry.y() << " w=" << trayGeometry.width()
                   << " h=" << trayGeometry.height();
    logger.debug() << "Screen geometry: x=" << screenGeometry.x()
                   << " y=" << screenGeometry.y()
                   << " w=" << screenGeometry.width()
                   << " h=" << screenGeometry.height();

    int x = trayGeometry.x() - m_window->width() / 2;
    int y = trayGeometry.y() - m_window->height() - 10;

    // Ensure the window stays within screen bounds
    if (x + m_window->width() > screenGeometry.right()) {
      x = screenGeometry.right() - m_window->width();
    }
    if (x < screenGeometry.left()) {
      x = screenGeometry.left();
    }
    if (y < screenGeometry.top()) {
      y = trayGeometry.bottom() + 10;
    }

    logger.debug() << "Setting window position to:" << x << "," << y;
    m_window->setPosition(x, y);
  } else {
    logger.warning() << "Tray icon geometry not valid, using default position";
    m_window->setPosition(100, 100);
  }

  logger.debug() << "Calling window show()";
  m_window->show();
  m_window->raise();
  m_window->requestActivate();

  logger.debug() << "Window visible status:" << m_window->isVisible();
  logger.debug() << "Window position: x=" << m_window->position().x()
                 << " y=" << m_window->position().y();
  logger.debug() << "Window size: w=" << m_window->size().width()
                 << " h=" << m_window->size().height();
}

void SysTray::hideWindow() {
  logger.debug() << "Hiding window";

  if (m_window) {
    m_window->hide();
  }
}
