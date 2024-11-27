/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxutils.h"

#include <mntent.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QtDBus/QtDBus>

#ifndef MZ_FLATPAK
#  include "dbusclient.h"
#endif
#include "logger.h"

#include "dbustypes.h"

namespace {
Logger logger("LinuxUtils");
}  // namespace

// static
QString LinuxUtils::findCgroupPath(const QString& type) {
  struct mntent entry;
  char buf[PATH_MAX];

  FILE* fp = fopen("/etc/mtab", "r");
  if (fp == NULL) {
    return QString();
  }

  while (getmntent_r(fp, &entry, buf, sizeof(buf)) != NULL) {
    if (strcmp(entry.mnt_type, "cgroup") != 0) {
      continue;
    }
    if (hasmntopt(&entry, type.toLocal8Bit().constData()) != NULL) {
      fclose(fp);
      return QString(entry.mnt_dir);
    }
  }
  fclose(fp);

  return QString();
}

// static
QString LinuxUtils::findCgroup2Path() {
  struct mntent entry;
  char buf[PATH_MAX];

  FILE* fp = fopen("/etc/mtab", "r");
  if (fp == NULL) {
    return QString();
  }

  while (getmntent_r(fp, &entry, buf, sizeof(buf)) != NULL) {
    if (strcmp(entry.mnt_type, "cgroup2") != 0) {
      continue;
    }
    return QString(entry.mnt_dir);
  }
  fclose(fp);

  return QString();
}

// static
QString LinuxUtils::gnomeShellVersion() {
  QDBusInterface iface("org.gnome.Shell", "/org/gnome/Shell",
                       "org.gnome.Shell");
  if (!iface.isValid()) {
    return QString();
  }

  QVariant shellVersion = iface.property("ShellVersion");
  if (!shellVersion.isValid()) {
    return QString();
  }
  return shellVersion.toString();
}

// static
QString LinuxUtils::kdeFrameworkVersion() {
  QProcess proc;
  proc.start("kf5-config", QStringList{"--version"}, QIODeviceBase::ReadOnly);
  if (!proc.waitForFinished()) {
    return QString();
  }

  QByteArray result = proc.readAllStandardOutput();
  for (const QByteArray& line : result.split('\n')) {
    if (line.startsWith("KDE Frameworks: ")) {
      return QString::fromUtf8(line.last(line.size() - 16));
    }
  }

  return QString();
}

// static
QString LinuxUtils::desktopFileId(const QString& path) {
  // Given the path to a .desktop file, return its Desktop File ID as per
  // the freedesktop.org's Desktop Entry Spec. See:
  // https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html#desktop-file-id
  //
  // To determine the ID of a desktop file, make its full path relative to the
  // $XDG_DATA_DIRS component in which the desktop file is installed, remove the
  // "applications/" prefix, and turn '/' into '-'.

  // If the path contains no slashes, assume this conversion is already done.
  if (!path.contains('/')) {
    return path;
  }

  // Find the application dir in the path.
  const QString dirComponent("/applications/");
  qsizetype index = path.lastIndexOf(dirComponent);
  if (index >= 0) {
    index += dirComponent.length();
  } else if (index < 0) {
    // If no applications dir was found, let's just use the filename.
    index = path.lastIndexOf('/') + 1;
    Q_ASSERT(index > 0);
  }

  // Convert it.
  return path.mid(index).replace('/', '-');
}

void LinuxUtils::setupAppScope(const QString& appId) {
  // Find our control group scope.
  QFile procCgroup("/proc/self/cgroup");
  if (!procCgroup.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text)) {
    logger.warning() << "Failed to open" << procCgroup.fileName();
    return;
  }
  QString cgroupv2;
  while (true) {
    QString line = QString::fromUtf8(procCgroup.readLine());
    if (line.isEmpty()) {
      // If we got to the end of the file without reading a cgroupsv2 scope then
      // cgroupsv2 probably isn't supported.
      return;
    }
    if (line.startsWith("0::")) {
      // We found the cgroupsv2
      cgroupv2 = line.mid(3);
      break;
    }
  }
  procCgroup.close();

  // Read the control group and parse out our systemd scope and appId.
  QString cgAppId;
  for (const QString& segment : cgroupv2.split('/')) {
    if (!segment.endsWith(".scope")) {
      continue;
    }
    QStringList cgScopeSplit = segment.first(segment.size()-5).split('-');
    if (cgScopeSplit[0] != "app") {
      continue;
    }
    // Remove the last element of the scope if it's a number.
    bool isDigit = false;
    cgScopeSplit.last().toULong(&isDigit);
    if (isDigit) {
      cgScopeSplit.removeLast();
    }
    // The appId should be the last remaining element of the scope.
    cgAppId = cgScopeSplit.last();
  }

  // If we found an appId then there is nothing to do.
  if (!cgAppId.isEmpty()) {
    logger.debug() << "Found AppId:" << cgAppId;
    return;
  }

  // Request a new scope from systemd via D-Bus
  QString newScope =
      QString("app-%1-%2.scope").arg(appId, QString::number(getpid()));
  QList<SystemdUnitProp> properties;
  QList<SystemdUnitAux> aux;
  QList<uint> pidlist({(uint)getpid()});
  properties.append(SystemdUnitProp("PIDs", QVariant::fromValue(pidlist)));

  logger.debug() << "Creating scope:" << newScope;

  QDBusInterface iface("org.freedesktop.systemd1", "/org/freedesktop/systemd1",
                       "org.freedesktop.systemd1.Manager");
  QDBusMessage msg = iface.call("StartTransientUnit", newScope, "fail",
                                QVariant::fromValue(properties), QVariant::fromValue(aux));
  if (msg.type() == QDBusMessage::ErrorMessage) {
    logger.warning() << "Failed to create transient unit:" << msg.errorMessage();
    return;
  }
  if (msg.type() != QDBusMessage::ReplyMessage) {
    logger.warning() << "Unexpected message type:" << msg.type();
    return;
  }
  QList<QVariant> results = msg.arguments();
  if (results.isEmpty()) {
    logger.warning() << "Empty response";
    return;
  }
  QDBusObjectPath path = results.first().value<QDBusObjectPath>();
  logger.info() << "Created app scope:" << path.path();
}
