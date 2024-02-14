/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKGETFEATURELIST_H
#define TASKGETFEATURELIST_H

#include <QObject>

#include "task.h"

/**
 * @brief Task to access the Guardian endpoint /featurelist.
 *
 * This is a POST endpoint. The request must either include the Authorization
 * header or the exerimenterId property in the body.
 *
 * When the user is logged in, the Authorization header will be included.
 *
 * When the user is logged out the value under `unauthedExperimenterId` will be
 * sent in the body.
 *
 * If the `unauthedExperimenterId` settings has never been set, this task
 * will set it.
 *
 */
class TaskGetFeatureList final : public Task {
  Q_DISABLE_COPY_MOVE(TaskGetFeatureList)

 public:
  TaskGetFeatureList();
  ~TaskGetFeatureList();

  void run() override;
};

#endif  // TASKGETFEATURELIST_H
