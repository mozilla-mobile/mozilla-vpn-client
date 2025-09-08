/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SYSTRAYWINDOW_H
#define SYSTRAYWINDOW_H

#include <QObject>
#include <QQuickWindow>

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

  QQuickWindow* m_window = nullptr;
};

#endif  // SYSTRAYWINDOW_H
