/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxutils.h"

#include <mntent.h>

#include <QDir>
#include <QDBusInterface>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QProcessEnvironment>
#include <QThread>
#include <QtDBus/QtDBus>

#ifndef MZ_FLATPAK
#  include "dbusclient.h"
#endif
#include "dbustypes.h"
#include "logger.h"

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

static QString decodeSystemdEscape(const QString& str) {
  static const QRegularExpression re("(_[0-9A-Fa-f][0-9A-Fa-f])");

  QString result = str;
  qsizetype offset = 0;
  while (offset < result.length()) {
    // Search for the next unicode escape sequence.
    QRegularExpressionMatch match = re.match(result, offset);
    if (!match.hasMatch()) {
      break;
    }

    bool okay;
    qsizetype start = match.capturedStart(0);
    QChar code = match.captured(0).mid(1).toUShort(&okay, 16);
    if (okay && (code != 0)) {
      // Replace the matched escape sequence with the decoded character.
      result.replace(start, match.capturedLength(0), QString(code));
      offset = start + 1;
    } else {
      // If we failed to decode the character, skip passed the matched string.
      offset = match.capturedEnd(0);
    }
  }

  return result;
}

void LinuxUtils::setupAppScope(const QString& appId) {
  // If we're in a sandbox environment, then it should already be setup.
  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  if (pe.contains("container")) {
    return;
  }

  uint ownPid = (uint)getpid();
  QDBusInterface sdManager("org.freedesktop.systemd1",
                           "/org/freedesktop/systemd1",
                           "org.freedesktop.systemd1.Manager");

  QDBusMessage getunit = sdManager.call("GetUnitByPID", ownPid);
  if (getunit.type() == QDBusMessage::ErrorMessage) {
    logger.warning() << "Failed to get scope:" << getunit.errorMessage();
    return;
  }
  QList<QVariant> result = getunit.arguments();
  if ((getunit.type() != QDBusMessage::ReplyMessage) || result.isEmpty()) {
    logger.warning() << "Bad reply for current scope:";
    return;
  }

  // Fetch the names of the unit to figure out the appid.
  QDBusObjectPath unitPath = result.first().value<QDBusObjectPath>();
  QDBusInterface ownUnit("org.freedesktop.systemd1", unitPath.path(),
                         "org.freedesktop.systemd1.Unit");
  QStringList unitNames = ownUnit.property("Names").toStringList();

  // Use the D-Bus object path as a fallback if there are no names.
  unitNames.append(decodeSystemdEscape(unitPath.path().split('/').last()));

  // Check to see if we have a valid application scope.
  QString unitAppId;
  for (const QString& name : unitNames) {
    if (!name.endsWith(".scope")) {
      continue;
    }
    QStringList scopeSplit = name.first(name.size() - 6).split('-');
    if (scopeSplit[0] != "app") {
      continue;
    }

    // Remove the last element of the scope if it's a number.
    bool isDigit = false;
    scopeSplit.last().toULong(&isDigit);
    if (isDigit) {
      scopeSplit.removeLast();
    }

    // The appId should be the last remaining element of the scope.
    // If we found an appId then we have no work to do.
    logger.info() << "Launched as app" << scopeSplit.last();
    return;
  }

  // Request a new scope from systemd via D-Bus
  QString newScopeName =
      QString("app-%1-%2.scope").arg(appId, QString::number(getpid()));
  SystemdUnitPropList properties;
  SystemdUnitAuxList aux;
  QList<uint> pidlist({ownPid});
  properties.append(SystemdUnitProp("PIDs", QVariant::fromValue(pidlist)));

  logger.debug() << "Creating scope:" << newScopeName;
  QDBusMessage msg =
      sdManager.call("StartTransientUnit", newScopeName, "fail",
                     QVariant::fromValue(properties), QVariant::fromValue(aux));
  if (msg.type() == QDBusMessage::ErrorMessage) {
    logger.warning() << "Failed to create scope:" << msg.errorMessage();
    return;
  }
  QDBusObjectPath jobPath = result.first().value<QDBusObjectPath>();
  if ((msg.type() != QDBusMessage::ReplyMessage) || jobPath.path().isEmpty()) {
    logger.warning() << "Bad reply for create scope";
    return;
  }

  // And now for the gross part. StartTransientUnit() returns a systemd job that
  // will create our scope for us, and move us into it. But we have to wait for
  // that job to finish before we can proceed. However, we can't do this async
  // because we don't have a QCoreApplication setup yet. The only way I can
  // think to do this is by polling systemd until the job is no longer running.
  QDBusInterface jobInterface("org.freedesktop.systemd1", jobPath.path(),
                              "org.freedesktop.systemd1.Job");
  while (jobInterface.isValid()) {
    QString state = jobInterface.property("State").toString();
    if (state.isEmpty()) {
      logger.debug() << "Job is done exiting...";
      break;
    }
    else if ((state != "waiting") && (state != "running")) {
      logger.debug() << "Job is" << state << "exiting...";
      break;
    }
    logger.debug() << "Job is" << state << "waiting...";
    QThread::currentThread()->wait(100);
  }
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
