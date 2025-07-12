/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxutils.h"

#include <mntent.h>

#include <QDBusInterface>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QProcessEnvironment>
#include <QThread>
#include <QtDBus/QtDBus>

#include "dbustypes.h"
#include "logger.h"

namespace {
Logger logger("LinuxUtils");
}  // namespace

// static
bool LinuxUtils::hasCgroupMount() {
  struct mntent entry;
  char buf[PATH_MAX];

  FILE* fp = fopen("/etc/mtab", "r");
  if (fp == NULL) {
    return false;
  }

  while (getmntent_r(fp, &entry, buf, sizeof(buf)) != NULL) {
    if (strcmp(entry.mnt_type, "cgroup") == 0) {
      return true;
    }
    if (strcmp(entry.mnt_type, "cgroup2") == 0) {
      return true;
    }
  }
  fclose(fp);
  return false;
}

// static
QVersionNumber LinuxUtils::gnomeShellVersion() {
  QDBusInterface iface("org.gnome.Shell", "/org/gnome/Shell",
                       "org.gnome.Shell");
  if (!iface.isValid()) {
    return QVersionNumber();
  }

  QVariant shellVersion = iface.property("ShellVersion");
  if (!shellVersion.isValid()) {
    return QVersionNumber();
  }
  return QVersionNumber::fromString(shellVersion.toString());
}

// static
QVersionNumber LinuxUtils::kdeFrameworkVersion() {
  QProcess proc;
  proc.start("kf5-config", QStringList{"--version"}, QIODeviceBase::ReadOnly);
  if (!proc.waitForFinished()) {
    return QVersionNumber();
  }

  QByteArray result = proc.readAllStandardOutput();
  for (const QByteArray& line : result.split('\n')) {
    if (line.startsWith("KDE Frameworks: ")) {
      auto vstr = QString::fromUtf8(line.last(line.size() - 16));
      return QVersionNumber::fromString(vstr);
    }
  }

  return QVersionNumber();
}

// Workaround for QTBUG-108822 by manually registering QDBusObjectPath with the
// D-Bus meta-type system, otherwise we are unable to connect to some signals.
#if QT_VERSION < 0x060403
class QtbugRegistrationProxy {
 public:
  QtbugRegistrationProxy() { qDBusRegisterMetaType<QDBusObjectPath>(); }
};

static QtbugRegistrationProxy s_qtbugRegistrationProxy;
#endif

// Ensure that the D-Bus custom types are registered.
static DBusMetatypeRegistrationProxy s_dbusMetatypeProxy;
