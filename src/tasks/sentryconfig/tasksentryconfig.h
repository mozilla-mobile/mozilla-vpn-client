
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKSENTRYCONFIG_H
#define TASKSENTRYCONFIG_H

#include <QByteArray>
#include <QObject>

#include "taskscheduler/task.h"

/**
 * @brief TaskSentryConfig fetches Sentry Configuration
 *
 * This Task will schedule a Network request to Guardians
 * `crashreporting` endpoint to get info on where to
 * post crashreports to.
 *
 * After a successful fetch, it will store the config
 * in the Settings an Call SentryAdapter::init to
 * complete activation of Sentry.
 *
 * This allows us to enable the opt-in crashreporter
 * also for clients built from source bundles that do
 * not have access to the Mozilla Build Secrets.
 *
 */

class TaskSentryConfig final : public Task {
  Q_DISABLE_COPY_MOVE(TaskSentryConfig)

 public:
  TaskSentryConfig();
  ~TaskSentryConfig();

  void run() override;

  DeletePolicy deletePolicy() const override { return Reschedulable; }

 private:
  /**
   * @brief Parses and stores a Sentry configuration.
   * Will call SentryAdapter::init(), if the configuration is
   * valid.
   *
   * @param data - A JSON blob containing {endpoint:URL, dsn:String}
   */
  static void parseSentryConfig(const QByteArray& data);
};

#endif  // TASKSENTRYCONFIG_H
