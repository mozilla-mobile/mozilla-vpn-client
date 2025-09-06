/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SYSTRAY_H
#define SYSTRAY_H

#include <QMenu>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QSystemTrayIcon>

class SysTray : public QObject {
  Q_OBJECT

 public:
  explicit SysTray(QObject* parent = nullptr);
  ~SysTray();

  void initialize();

 private slots:
  void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
  void showWindow();
  void hideWindow();

 private:
  void createTrayIcon();
  void createContextMenu();
  void setupWindow();

  QSystemTrayIcon* m_trayIcon = nullptr;
  QMenu* m_trayMenu = nullptr;
  QQuickWindow* m_window = nullptr;
};

#endif  // SYSTRAY_H
