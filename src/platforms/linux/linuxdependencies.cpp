/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxdependencies.h"

#include <mntent.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

#include "dbusclient.h"
#include "logger.h"

namespace {

Logger logger("LinuxDependencies");

void showAlert(const QString& message) {
  logger.debug() << "Show alert:" << message;

  QMessageBox alert;
  alert.setText(message);
  alert.exec();
}

bool checkDaemonVersion() {
  logger.debug() << "Check Daemon Version";

  DBusClient* dbus = new DBusClient(nullptr);
  QDBusPendingCallWatcher* watcher = dbus->version();

  bool completed = false;
  bool value = false;
  QObject::connect(
      watcher, &QDBusPendingCallWatcher::finished, watcher,
      [completed = &completed, value = &value](QDBusPendingCallWatcher* call) {
        *completed = true;

        QDBusPendingReply<QString> reply = *call;
        if (reply.isError()) {
          logger.error() << "DBus message received - error";
          *value = false;
          return;
        }

        QString version = reply.argumentAt<0>();
        *value = version == PROTOCOL_VERSION;

        logger.debug() << "DBus message received - daemon version:" << version
                       << " - current version:" << PROTOCOL_VERSION;
      });

  while (!completed) {
    QCoreApplication::processEvents();
  }

  delete dbus;
  return value;
}

}  // namespace

// static
bool LinuxDependencies::checkDependencies() {
  char* path = getenv("PATH");
  if (!path) {
    showAlert("No PATH env found.");
    return false;
  }

  if (!checkDaemonVersion()) {
    showAlert("mozillavpn linuxdaemon needs to be updated or restarted.");
    return false;
  }

  return true;
}

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
