/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SETTINGSWATCHER_H
#define SETTINGSWATCHER_H

#include <QObject>

/**
 * @brief this class watches a few setting properties to see if we need to
 * trigger a silent-server-switch
 */
class SettingsWatcher final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(SettingsWatcher)

 public:
  static SettingsWatcher* instance();

  ~SettingsWatcher();

 private:
  explicit SettingsWatcher(QObject* parent);

  void maybeServerSwitch();

  void operationCompleted();

 private:
  class TaskSettingsWatcher;

  bool m_operationRunning = false;
};

#endif  // SETTINGSWATCHER_H
