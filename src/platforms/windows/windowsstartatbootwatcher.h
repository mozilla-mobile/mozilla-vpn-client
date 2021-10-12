/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSSTARTATBOOTWATCHER_H
#define WINDOWSSTARTATBOOTWATCHER_H

#include <QObject>

class WindowsStartAtBootWatcher final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(WindowsStartAtBootWatcher)

 public:
  explicit WindowsStartAtBootWatcher(bool startAtBoot);
  ~WindowsStartAtBootWatcher();

  void startAtBootChanged(const bool& startAtBoot);
};

#endif  // WINDOWSSTARTATBOOTWATCHER_H
