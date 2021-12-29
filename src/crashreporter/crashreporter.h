/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CRASHREPORTER_H
#define CRASHREPORTER_H

#include <QObject>
#include <memory>

#include "crashui.h"
#include "qmlengineholder.h"
#include "settingsholder.h"

class CrashData;

class CrashReporter : public QObject {
  Q_OBJECT
 public:
  explicit CrashReporter(QObject* parent = nullptr);
  virtual bool start(int argc, char* argv[]) = 0;
  virtual void stop(){};
  virtual bool shouldPromptUser();
  bool promptUser();

 private:
  std::unique_ptr<CrashUI> m_ui;
  QmlEngineHolder m_engineHolder;
  SettingsHolder settings;
};

#endif  // CRASHREPORTER_H
