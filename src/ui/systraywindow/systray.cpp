/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QApplication>
#include <QQmlComponent>
#include <QQmlContext>
#include <QScreen>

#include "qmlengineholder.h"
#include "systraywindow.h"
#include "utils/leakdetector.h"
#include "utils/logger.h"

namespace {
Logger logger("SysTrayWindow");
}

SysTrayWindow::SysTrayWindow(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(SysTrayWindow);
}

SysTrayWindow::~SysTrayWindow() {
  MZ_COUNT_DTOR(SysTrayWindow);

  if (m_window) {
    m_window->deleteLater();
  }
}

void SysTrayWindow::setupWindow() {
  logger.debug() << "Setting up QML window";

  auto engine = QmlEngineHolder::instance()->engine();

  // Load the QML component from the qt_add_qml_module generated path
  QQmlComponent component(engine,
                          QUrl("qrc:/qt/qml/systraywindow/systraywindow.qml"));

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

QPoint SysTrayWindow::calculateWindowPosition() {
  if (!m_window) {
    logger.warning() << "Cannot calculate position - window is null";
    return QPoint(100, 100);
  }

  QScreen* screen = QApplication::primaryScreen();
  QRect screenGeometry = screen->geometry();
  QRect availableGeometry = screen->availableGeometry();

  logger.debug() << "Screen geometry: (" << screenGeometry.x() << ","
                 << screenGeometry.y() << " " << screenGeometry.width() << "x"
                 << screenGeometry.height() << ")";
  logger.debug() << "Available geometry: (" << availableGeometry.x() << ","
                 << availableGeometry.y() << " " << availableGeometry.width()
                 << "x" << availableGeometry.height() << ")";

  int windowWidth = m_window->width();
  int windowHeight = m_window->height();
  int x, y;

  // Determine taskbar position by comparing screen and available geometry
  bool taskbarBottom = availableGeometry.bottom() < screenGeometry.bottom();
  bool taskbarTop = availableGeometry.top() > screenGeometry.top();
  bool taskbarLeft = availableGeometry.left() > screenGeometry.left();
  bool taskbarRight = availableGeometry.right() < screenGeometry.right();

  if (taskbarBottom) {
    // Taskbar at bottom - position window above it, centered horizontally
    x = availableGeometry.left() +
        (availableGeometry.width() - windowWidth) / 2;
    y = availableGeometry.bottom() - windowHeight - 10;
    logger.debug() << "Taskbar detected at bottom";
  } else if (taskbarTop) {
    // Taskbar at top - position window below it, centered horizontally
    x = availableGeometry.left() +
        (availableGeometry.width() - windowWidth) / 2;
    y = availableGeometry.top() + 10;
    logger.debug() << "Taskbar detected at top";
  } else if (taskbarLeft) {
    // Taskbar at left - position window at bottom-right
    x = availableGeometry.right() - windowWidth - 10;
    y = availableGeometry.bottom() - windowHeight - 10;
    logger.debug() << "Taskbar detected at left";
  } else if (taskbarRight) {
    // Taskbar at right - position window at bottom, away from taskbar
    x = availableGeometry.right() - windowWidth - 10;
    y = availableGeometry.bottom() - windowHeight - 10;
    logger.debug() << "Taskbar detected at right";
  } else {
    // Default case - bottom center
    x = availableGeometry.left() +
        (availableGeometry.width() - windowWidth) / 2;
    y = availableGeometry.bottom() - windowHeight - 10;
    logger.debug() << "No taskbar detected, using default position";
  }

  // Ensure window stays within available area
  if (x < availableGeometry.left()) {
    x = availableGeometry.left() + 10;
  }
  if (y < availableGeometry.top()) {
    y = availableGeometry.top() + 10;
  }
  if (x + windowWidth > availableGeometry.right()) {
    x = availableGeometry.right() - windowWidth - 10;
  }
  if (y + windowHeight > availableGeometry.bottom()) {
    y = availableGeometry.bottom() - windowHeight - 10;
  }

  logger.debug() << "Calculated window position:" << x << "," << y;
  return QPoint(x, y);
}

void SysTrayWindow::showWindow() {
  logger.debug() << "Showing window";

  // Create window on-demand if it doesn't exist
  if (!m_window) {
    setupWindow();
  }

  if (!m_window) {
    logger.error() << "Cannot show window - failed to create window";
    return;
  }

  logger.debug() << "Window is valid, proceeding to show";

  // Calculate and set window position
  QPoint position = calculateWindowPosition();
  logger.debug() << "Setting window position to:" << position.x() << ","
                 << position.y();
  m_window->setPosition(position);

  logger.debug() << "Calling window show()";
  m_window->show();
  m_window->raise();
  m_window->requestActivate();

  logger.debug() << "Window visible status:" << m_window->isVisible();
}

void SysTrayWindow::hideWindow() {
  logger.debug() << "Hiding window";

  if (m_window) {
    m_window->hide();
  }
}
