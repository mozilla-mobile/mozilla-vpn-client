/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APPTRACKER_H
#define APPTRACKER_H

#include <QFileSystemWatcher>
#include <QHash>
#include <QString>

#include "leakdetector.h"

class QDBusInterface;

class AppTracker final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AppTracker)

 public:
  explicit AppTracker(QObject* parent = nullptr);
  ~AppTracker();

  void userCreated(uint userid, const QString& xdgRuntimePath);
  void userRemoved(uint userid);

  QStringList getRunningApps() const { return m_runningApps.keys(); }
  QString getDesktopId(const QString& cgroup) const {
    return m_runningApps.value(cgroup);
  }
  QStringList findByDesktopId(const QString& desktopId) const {
    return m_runningApps.keys(desktopId);
  }

 signals:
  void appLaunched(const QString& cgroup, const QString& desktopId);
  void appTerminated(const QString& cgroup, const QString& desktopId);

 private slots:
  void cgroupsChanged(const QString& directory);

 private:
  QString findDesktopId(const QString& cgroup);
  QList<int> pids(const QString& cgroup) const;
  static QString decodeUnicodeEscape(const QString& str);

 private:
  // Monitoring of the user's control groups.
  QString m_cgroupMount;
  QFileSystemWatcher m_cgroupWatcher;
  QDBusInterface* m_systemdInterface = nullptr;

  // The set of applications that we have tracked.
  //  - key is the cgroup path.
  //  - value is the desktopId, or an empty QString if unknown.
  QHash<QString, QString> m_runningApps;
};

#endif  // APPTRACKER_H
