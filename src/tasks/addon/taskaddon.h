/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKADDON_H
#define TASKADDON_H

#include "task.h"

#include <QObject>

class TaskAddon final : public Task {
  Q_DISABLE_COPY_MOVE(TaskAddon)

 public:
  TaskAddon(const QString& addonId, const QByteArray& sha256);
  ~TaskAddon();

  void run() override;

  // If we cancel this task, we will not know if the add-ons have been
  // downloaded and when they are ready to be loaded.
  bool deletable() const override { return false; }

 private:
  const QString m_addonId;
  const QByteArray m_sha256;
};

#endif  // TASKADDON_H
