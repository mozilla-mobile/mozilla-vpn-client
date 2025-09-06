/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SYSTRAYMANAGER_H
#define SYSTRAYMANAGER_H

#include <QObject>

#include "systray.h"

class SysTrayManager : public QObject {
  Q_OBJECT

 public:
  static SysTrayManager* instance();

  Q_INVOKABLE void showSysTray();
  Q_INVOKABLE void hideSysTray();
  Q_INVOKABLE bool isInitialized() const;

 public slots:
  void initialize();

 private:
  explicit SysTrayManager(QObject* parent = nullptr);
  ~SysTrayManager();

  static SysTrayManager* s_instance;
  SysTray* m_sysTray = nullptr;
};

#endif  // SYSTRAYMANAGER_H
