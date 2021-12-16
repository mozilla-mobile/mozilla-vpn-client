/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CRASHREPORTER_H
#define CRASHREPORTER_H

#include <QObject>
#include <memory>

#ifdef MVPN_WINDOWS
#  include <WinSock2.h>
// clang-format off
#  pragma include_alias(<sys/time.h>, <time.h>)
// clang-format on
#endif

#include <snapshot/process_snapshot.h>
#include "crashdata.h"
#include "crashui.h"

class CrashData;

class CrashReporter : public QObject {
  Q_OBJECT
 public:
  explicit CrashReporter(QObject* parent = nullptr);
  virtual bool start(int argc, char* argv[]) = 0;
  virtual void stop(){};
  virtual bool shouldPromptUser();
  bool promptUser();
 public slots:
  void crashReported(std::shared_ptr<CrashData> data);

 protected:
  virtual std::shared_ptr<crashpad::ProcessSnapshot> createSnapshot(
      std::shared_ptr<CrashData> data) = 0;

 private:
  std::unique_ptr<CrashUI> m_ui;
};

#endif  // CRASHREPORTER_H
