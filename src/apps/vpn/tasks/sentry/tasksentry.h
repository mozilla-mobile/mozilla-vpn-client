
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKSENTRY_H
#define TASKSENTRY_H

#include <QByteArray>
#include <QObject>

#include "task.h"

class TaskSentry final : public Task {
  Q_DISABLE_COPY_MOVE(TaskSentry)

 public:
  TaskSentry(const QByteArray& envelope);
  ~TaskSentry();

  void run() override;

 private:
  /**
   * @brief Creates a network request to send the envelope
   *
   */
  void sendRequest();

  /**
   * @brief Checks if the data contains a crash report
   *
   */
  bool isCrashReport();

  /**
   * Reads mEnvelope and fills interal fields,
   * might be costly.
   */
  void parseEnvelope();

  QByteArray m_envelope;
  bool m_checkedContent = false;
  bool m_isCrashReport = false;
  QString m_eventID;
};

#endif  // TASKSENTRY_H