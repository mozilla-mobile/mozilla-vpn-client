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

// Applications on Linux can be a bit vague and hard to define at runtime, so
// we need to make some assumptions to try and tackle the problem.
//
// The first concept, comes from the Freedesktop specification, which makes use
// of desktop entry files to populate entities in the applications menu. The
// path to these files are used to generate identifiers, known as Desktop File
// IDs. This means that we can only track applications which are launched by
// the applications menu.
//
// The second contept are control groups, or cgroups, which are used to group
// process IDs together for the purpose of establishing shared resource
// contstraints. We use these control groups in the Linux netfilter subsystem
// to apply the split tunnelling traffic manipulation.
//
// It just so happens that many modern desktop environments will group their
// processes forked from the application launchers into cgroups for resource
// management and containerization. This class attempts to track those cgroups
// and match them to the destop file ID from which they originated.
//
// This means that we also only support desktop environments which make us of
// control groups for application containerization. This more or less limits us
// to Gnome, KDE, Flatpaks and Snaps.
class AppTracker final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AppTracker)

 public:
  explicit AppTracker(QObject* parent = nullptr);
  ~AppTracker();

  /**
   * @brief Track a new user session for control group scopes where applications
   *        may be running.
   *
   * @param userid Unix User identifier.
   * @param xdgRuntimePath 
   */
  void userCreated(uint userid, const QString& xdgRuntimePath);

  /**
   * @brief Terminate tracking of a user session.
   *
   * @param userid Unix User identifier.
   */
  void userRemoved(uint userid);

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

 signals:
  void appLaunched(const QString& cgroup, const QString& desktopFileId);
  void appTerminated(const QString& cgroup, const QString& desktopFileId);

 private slots:
  void cgroupsChanged(const QString& directory);

 private:
  QString findDesktopFileId(const QString& cgroup);
  static QString snapDesktopFileId(const QString& cgroup);
  static QString decodeUnicodeEscape(const QString& str);

 private:
  // Monitoring of the user's control groups.
  QString m_cgroupMount;
  QFileSystemWatcher m_cgroupWatcher;
  QDBusInterface* m_systemdInterface = nullptr;

  // The set of control groups that are currently running, and the desktop file
  // IDs to which we have mapped them. The key to this QHash is the control
  // group path, and the value is the mapped desktop file ID, or an empty
  // QString if unknown.
  QHash<QString, QString> m_runningCgroups;
};

#endif  // APPTRACKER_H
