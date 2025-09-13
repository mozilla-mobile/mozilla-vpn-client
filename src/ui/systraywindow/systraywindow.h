/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SYSTRAYWINDOW_H
#define SYSTRAYWINDOW_H

#include <QObject>
#include <QPoint>
#include <QQuickWindow>

/**
 * @brief Manages the system tray popup window
 *
 * This class handles the creation and display of a QML-based popup window
 * that appears when the system tray icon is clicked. The window is created
 * on-demand and can be shown/hidden as needed.
 */
class SysTrayWindow : public QObject {
  Q_OBJECT

 public:
  explicit SysTrayWindow(QObject* parent = nullptr);
  ~SysTrayWindow();

 public slots:
  void showWindow();
  void hideWindow();

 private:
  void setupWindow();
  QPoint calculateWindowPosition();

  QQuickWindow* m_window = nullptr;
};

#endif  // SYSTRAYWINDOW_H
