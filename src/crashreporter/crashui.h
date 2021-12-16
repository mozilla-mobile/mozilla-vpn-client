/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CRASHUI_H
#define CRASHUI_H

#include <QQmlApplicationEngine>
#include <memory>

class CrashUI {
 public:
  CrashUI();
  void initialize();
  void showUI();

 private:
  std::unique_ptr<QQmlApplicationEngine> m_engine;
  bool m_initialized = false;
};

#endif  // CRASHUI_H
