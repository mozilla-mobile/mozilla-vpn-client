/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MODULE_H
#define MODULE_H

#include <QJsonObject>
#include <QObject>

#include "notificationhandler.h"

class QTextStream;
class Task;
class TutorialStepBefore;

class Module : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Module)

 public:
  static void load(QObject* parent);

  explicit Module(QObject* parent);
  virtual ~Module();

  /**
   * @brief Initialize the module
   */
  virtual void initialize() = 0;

  /**
   * @brief Inform the module that it's the right time to read from settings
   */
  virtual void settingsAvailable() {}

  /**
   * @brief the client needs to update.
   *
   * This method needs to trigger the signal `readyToUpdate`.
   */
  virtual void updateRequired() = 0;

  /**
   * @brief The client needs to quit.
   *
   * This method needs to trigger the signal `readyToQuit`.
   */
  virtual void quit() = 0;

  /**
   * @brief The is a backend failure
   *
   * This method needs to trigger the signal `readyToBackendFailure`.
   */
  virtual void backendFailure() = 0;

  /**
   * @brief Retrieve extra logs for this module.
   */
  virtual void serializeLogs(QTextStream* out,
                             std::function<void()>&& a_finalizeCallback) = 0;

  /**
   * @brief Clean up all the log entries
   */
  virtual void cleanupLogs() = 0;

  /**
   * @brief Create a tutorial step before object if supported
   *
   * @param parent - the parent object to use
   * @param name - the name of this step
   * @param json - the JSON object to parse
   */
  virtual TutorialStepBefore* maybeCreateTutorialStepBefore(
      QObject* parent, const QString& name, const QJsonObject& json) {
    return nullptr;
  }

  /**
   * @brief retrieve a list of periodic tasks
   */
  virtual QList<Task*> retrievePeriodicTasks() { return QList<Task*>(); }

 signals:
  void readyToUpdate();
  void readyToQuit();
  void readyToBackendFailure();

  void notificationNeeded(NotificationHandler::Message type,
                          const QString& title, const QString& message,
                          int timerMsec);

  void serverConnectionMessage(const QJsonObject& obj);
};

#endif  // MODULE_H
