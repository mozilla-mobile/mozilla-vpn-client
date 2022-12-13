/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKCREATESUPPORTTICKET_H
#define TASKCREATESUPPORTTICKET_H

#include <QObject>

#include "task.h"

class TaskCreateSupportTicket final : public Task {
  Q_DISABLE_COPY_MOVE(TaskCreateSupportTicket)

 public:
  TaskCreateSupportTicket(const QString& email, const QString& subject,
                          const QString& issueText, const QString& logs,
                          const QString& category);
  ~TaskCreateSupportTicket();

  void run() override;

 private:
  QString m_email;
  QString m_subject;
  QString m_issueText;
  QString m_logs;
  QString m_category;
};

#endif  // TASKCREATESUPPORTTICKET_H
