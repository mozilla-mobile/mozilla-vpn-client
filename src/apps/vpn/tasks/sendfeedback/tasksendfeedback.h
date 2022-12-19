/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKSENDFEEDBACK_H
#define TASKSENDFEEDBACK_H

#include <QObject>

#include "task.h"

class TaskSendFeedback final : public Task {
  Q_DISABLE_COPY_MOVE(TaskSendFeedback)

 public:
  explicit TaskSendFeedback(const QString& feedbackText, const QString& logs,
                            const qint8 rating, const QString& category);
  ~TaskSendFeedback();

  void run() override;

 private:
  QString m_feedbackText;
  QString m_logs;
  qint8 m_rating;
  QString m_category;
};

#endif  // TASKSENDFEEDBACK_H
