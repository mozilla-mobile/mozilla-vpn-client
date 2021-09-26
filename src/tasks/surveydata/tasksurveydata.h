/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKSURVEYDATA_H
#define TASKSURVEYDATA_H

#include "task.h"

#include <QObject>

class TaskSurveyData final : public Task {
  Q_DISABLE_COPY_MOVE(TaskSurveyData)

 public:
  TaskSurveyData();
  ~TaskSurveyData();

  void run(Core* core) override;
};

#endif  // TASKSURVEYDATA_H
