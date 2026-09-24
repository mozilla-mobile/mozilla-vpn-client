/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APPTRACKER_H
#define APPTRACKER_H

#include <QDBusContext>
#include <QFileSystemWatcher>
#include <QHash>
#include <QString>

#include "leakdetector.h"

class QDBusError;
class QDBusInterface;
class QDBusObjectPath;
class QDBusVariant;

// Applications on Linux can be a bit vague and hard to define at runtime, so
// we need to make some assumptions to try and tackle the problem.
//
// First off, we try to identify applications based on the application menu
// entries. According to the Freedesktop specification, these are described by
// the `*.desktop` files found under the user's XDG_DATA_DIRS environment
// variable. The Freedesktop specification also defines that the path to this
// file can be converted into a Desktop File ID, which we will use as the
// identifier of the application.
//
// However, the `*.desktop` files only describe how to launch an application.
// Once an application has started, there is no definitive way to track the
// processes of that application. For this, we rely on Linux control groups,
// or cgroups, which are used to group processes together for the purpose of
// establishing shared resource constraints and containerization.
//
// It just so happens that many modern desktop environments will group their
// processes forked from the application launchers into cgroups for resource
// management and containerization. This class attempts to track those cgroups
// and match them to the destop file ID from which they originated.
//
// This means that we only support application environments which make use of
// control groups for application containerization, and apps which can be
// launched via the applications menu. This limits us to Gnome, KDE, Flatpaks,
// and Snaps.
class AppTracker final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AppTracker)

 public:
  explicit AppTracker(const QString& path, QObject* parent = nullptr);
  ~AppTracker();

  /**
   * @brief Return a list of control groups matching a given desktop file ID.
   *
   * @param desktopFileId desktop file ID to match.
   *
   * @returns a list control group scopes.
   */
  QStringList findByDesktopFileId(const QString& desktopFileId) const {
    return m_runningCgroups.keys(desktopFileId);
  }

  uint userId() const { return m_userId; }
  const QStringList appControlGroups() const { return m_runningCgroups.keys(); }
  const QString& userObjectPath() const { return m_userObject; }
  const QString& userControlGroup() const { return m_userCgroup; }

 signals:
  void appLaunched(const QString& cgroup, const QString& desktopFileId);
  void appTerminated(const QString& cgroup, const QString& desktopFileId);

 private slots:
  void cgroupCreated(const QString& cgroup);
  void cgroupsChanged(const QString& directory);
  void dbusErrorOccurred(const QDBusError& err);
  void userRuntimeFinished(const QDBusVariant& props);
  void userCgroupFinished(const QDBusVariant& cgroup);

 private:
  static QString snapDesktopFileId(const QString& cgroup);
  QString path2cgroup(const QString& path) const;
  void cgroupResolved(const QString& cgroup, const QString& desktopFileId);

  void userFetch();
  void userConnect();

 private:
  // D-Bus connection name to the user's D-Bus session.
  QString m_connectionName;

  // Systemd login session.
  const QString m_userObject;
  QString m_userCgroup;
  QString m_userSocket;
  uint m_userId = 0;

  // Monitoring of the user's control groups.
  QString m_cgroupMount;
  QFileSystemWatcher m_cgroupWatcher;

  // The set of control groups that are currently running, and the desktop file
  // IDs to which we have mapped them. The key to this QHash is the control
  // group path, and the value is the mapped desktop file ID, or an empty
  // QString if unknown.
  QHash<QString, QString> m_runningCgroups;
};

// A helper class to perform the KDE fallback asynchronously, which attempts
// to fetch the systemd unit for a cgroup and then get the SourcePath property.
class KdeFallbackTracker : public QObject, protected QDBusContext {
  Q_OBJECT

 public:
  KdeFallbackTracker(const QString& cgroup, const QDBusConnection& connection,
                     QObject* parent = nullptr);

  const QString& cgroup() const { return m_cgroup; }

 signals:
  void errorOccurred(const QDBusError& err);
  void finished(const QString& cgroup, const QString& desktopFileId);

 private slots:
  void unitLookupFinished(const QDBusObjectPath& unit);
  void sourceLookupFinished(const QDBusVariant& source);

 private:
  const QString m_cgroup;
};

#endif  // APPTRACKER_H
