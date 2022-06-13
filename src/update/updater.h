/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>

class Task;

class Updater : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Updater)

 public:
  enum Step {
    RecommendedUpdateAvailable,
    RequiredUpdateAvailable,
    UpdateViewShown,
    UpdateProcessStarted,
    FallbackInBrowser,
    BalrogValidationCompleted,
    BalrogFileSaved,
    InstallationProcessExecuted,
    ApplicationRestartedAfterUpdate,
  };
  Q_ENUM(Step);

  static Updater* create(QObject* parent, bool downloadAndInstall);

  static void updateViewShown();

  Updater(QObject* parent);
  virtual ~Updater();

  virtual void start(Task* task) = 0;

  bool recommendedOrRequired() const { return m_recommendedOrRequired; }

 signals:
  void updateRequired();
  void updateRecommended();

 protected:
  static QString appVersion();

 private:
  bool m_recommendedOrRequired = false;
};

#endif  // UPDATER_H
