/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MODULE_H
#define MODULE_H

#include "notificationhandler.h"

#include <QObject>

class QTextStream;

class Module : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Module)

 public:
  static void load(QObject* parent);

  explicit Module(QObject* parent);
  virtual ~Module();

  virtual void initialize() = 0;

  // This method needs to trigger the signal `readyToUpdate`.
  virtual void updateRequired() = 0;

  // This method needs to trigger the signal `readyToQuit`.
  virtual void quit() = 0;

  // This method needs to trigger the signal `readyToBackendFailure`.
  virtual void backendFailure() = 0;

  // Retrieve extra logs for this module.
  virtual void serializeLogs(QTextStream* out,
                             std::function<void()>&& a_finalizeCallback) = 0;

  virtual void cleanupLogs() = 0;

 signals:
  void readyToUpdate();
  void readyToQuit();
  void readyToBackendFailure();

  void notificationNeeded(NotificationHandler::Message type,
                          const QString& title, const QString& message,
                          int timerMsec);
};

#endif  // MODULE_H
