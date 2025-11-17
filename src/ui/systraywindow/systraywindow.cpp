/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systraywindow.h"

#include <QApplication>
#include <QQmlComponent>
#include <QQmlContext>
#include <QScreen>

#include "qmlengineholder.h"
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
                          QUrl("qrc:/qt/qml/systraywindow/SystrayWindow.qml"));

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
  QRect screenGeometry = screen->availableGeometry();

  // Position window in bottom-right corner with some margin
  int x = screenGeometry.right() - m_window->width() - 20;
  int y = screenGeometry.bottom() - m_window->height() - 50;

  // Ensure window stays within screen bounds
  if (x < screenGeometry.left()) {
    x = screenGeometry.left() + 10;
  }
  if (y < screenGeometry.top()) {
    y = screenGeometry.top() + 10;
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
