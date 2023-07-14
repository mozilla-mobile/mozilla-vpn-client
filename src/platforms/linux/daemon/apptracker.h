/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APPTRACKER_H
#define APPTRACKER_H

#include <QDBusObjectPath>
#include <QFileSystemWatcher>
#include <QString>

#include "leakdetector.h"

class QDBusInterface;

class AppData {
 public:
  AppData(const QString& path) : cgroup(path) { MZ_COUNT_CTOR(AppData); }
  ~AppData() { MZ_COUNT_DTOR(AppData); }

  QList<int> pids() const;

  const QString cgroup;
  QString appId;
  int rootpid = 0;
};

class AppTracker final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AppTracker)

 public:
  explicit AppTracker(QObject* parent = nullptr);
  ~AppTracker();

  void userCreated(uint userid, const QDBusObjectPath& path);
  void userRemoved(uint userid, const QDBusObjectPath& path);

  QHash<QString, AppData*>::iterator begin() { return m_runningApps.begin(); }
  QHash<QString, AppData*>::iterator end() { return m_runningApps.end(); }
  AppData* find(const QString& cgroup) { return m_runningApps.value(cgroup); }

 signals:
  void appLaunched(const QString& cgroup, const QString& appId, int rootpid);
  void appTerminated(const QString& cgroup, const QString& appId);

 private slots:
  void gtkLaunchEvent(const QByteArray& appid, const QString& display,
                      qlonglong pid, const QStringList& uris,
                      const QVariantMap& extra);

  void cgroupsChanged(const QString& directory);

 private:
  void appHeuristicMatch(AppData* data);

 private:
  // Monitoring of the user's control groups.
  QString m_cgroupMount;
  QFileSystemWatcher m_cgroupWatcher;
  QDBusInterface* m_systemdInterface = nullptr;

  // The set of applications that we have tracked.
  QHash<QString, AppData*> m_runningApps;
  QString m_lastLaunchName;
  int m_lastLaunchPid;
};

#endif  // APPTRACKER_H
