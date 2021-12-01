/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKINITIALIZEADJUST_H
#define TASKINITIALIZEADJUST_H

#include "task.h"

#include <QObject>

class TaskInitializeAdjust final : public Task {
  Q_DISABLE_COPY_MOVE(TaskInitializeAdjust)

 public:
  TaskInitializeAdjust();
  ~TaskInitializeAdjust();

  void run() override;
};

#endif  // TASKINITIALIZEADJUST_H
