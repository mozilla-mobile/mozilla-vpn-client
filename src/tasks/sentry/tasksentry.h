
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKSENTRY_H
#define TASKSENTRY_H

#include <QByteArray>
#include <QObject>

#include "sentry/sentrysniffer.h"
#include "task.h"

// Task to get user consent and send crash report to
// Sentry (https://github.com/getsentry/sentry-native)
class TaskSentry final : public Task {
  Q_DISABLE_COPY_MOVE(TaskSentry)

 public:
  TaskSentry(const QByteArray& envelope);
  ~TaskSentry();

  void run() override;

  // Reschedulable because user needs to be prompted always to send crash
  // reports
  DeletePolicy deletePolicy() const override {
    return DeletePolicy::Reschedulable;
  }

 private:
  /**
   * @brief Creates a network request to send the envelope
   *
   */
  void sendRequest();

  QByteArray m_envelope;
  SentrySniffer::ContentType m_Type = SentrySniffer::ContentType::Unknown;
  QString m_eventID;
};

#endif  // TASKSENTRY_H
