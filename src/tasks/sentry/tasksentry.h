
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKSENTRY_H
#define TASKSENTRY_H

#include <QByteArray>
#include <QObject>

#include "taskscheduler/task.h"

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

  enum ContentType { Unknown, Ping, CrashReport };
  Q_ENUM(ContentType);

  /**
   * Reads the sentry-envelope and checks
   * if it is a Ping or CrashReport
   */
  TaskSentry::ContentType parseEnvelope();

 private:
  /**
   * @brief Creates a network request to send the envelope
   *
   */
  void sendRequest();

  /**
   * @brief Checks if a Sentry header Obj contains
   * a crash report
   * @param obj - The json object to chekc
   * @return true - crashdata exists
   * @return false - is something else
   */
  static bool isCrashReportHeader(const QJsonObject& obj);

  QByteArray m_envelope;
  TaskSentry::ContentType m_Type = TaskSentry::ContentType::Unknown;
  QString m_eventID;
};

#endif  // TASKSENTRY_H
