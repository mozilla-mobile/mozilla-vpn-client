/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TASKGETFEATURELISTWORKER_H
#define TASKGETFEATURELISTWORKER_H

#include <QObject>
#include <QTimer>

/**
 * @brief Worker responsible for executing TaskGetFeatureList
 * at the appropriate times.
 *
 * What are the appropriate times?
 *
 * * On initialization
 * * Whenever the experimenter id changes -- the feature list depends on it
 * * Periodically on a specified interval
 *
 * What is the experimenter id?
 *
 * The experimenter id is an id associated to a user session and used by Cirrus
 * to determine which experimental features should be enabled for this user.
 *
 * On a logged out session of the VPN app, that will be a randomly generated
 * UUID stored under the unauthedExperimenterId setting.
 *
 * On a logged in session of the VPN app, that will be the FxA id which is not
 * passed in the request in plain text, but is gleaned from the JWT user token
 * sent to the server through the Authorization header.
 */
class TaskGetFeatureListWorker final : public QObject {
  Q_OBJECT

 public:
  TaskGetFeatureListWorker(QObject* parent = nullptr);
  ~TaskGetFeatureListWorker();

  /**
   * @brief Starts the worker.
   *
   * 1. Schedules TaskGetFeatureList to run ASAP
   * 2. Starts a timer to re-run TaskGetFeatureList on a given interval
   * 3. Starts observers for re-triggering the task on experimenter id change
   */
  void start(std::chrono::milliseconds interval);

 private:
  void scheduleTask();

 private:
  QTimer m_timer;
};

#endif  // TASKGETFEATURELISTWORKER_H
