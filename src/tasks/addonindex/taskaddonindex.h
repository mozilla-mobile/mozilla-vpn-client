/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKADDONINDEX_H
#define TASKADDONINDEX_H

#include "task.h"

#include <QObject>

class TaskAddonIndex final : public Task {
  Q_DISABLE_COPY_MOVE(TaskAddonIndex)

 public:
  TaskAddonIndex();
  ~TaskAddonIndex();

  void run() override;

 private:
  void maybeComplete();

 private:
  QByteArray m_indexData;
  QByteArray m_indexSignData;
};

#endif  // TASKADDONINDEX_H
