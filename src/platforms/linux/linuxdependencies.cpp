/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxdependencies.h"

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

namespace {
Logger logger("LinuxDependencies");
}  // namespace

// static
QString LinuxDependencies::findCgroupPath(const QString& type) {
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
QString LinuxDependencies::findCgroup2Path() {
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
QString LinuxDependencies::gnomeShellVersion() {
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
QString LinuxDependencies::kdeFrameworkVersion() {
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
QString LinuxDependencies::desktopFileId(const QString& path) {
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
