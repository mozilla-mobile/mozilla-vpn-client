/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxdependencies.h"
#include "dbusclient.h"
#include "logger.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

constexpr const char* WG_QUICK = "wg-quick";

namespace {

Logger logger(LOG_LINUX, "LinuxDependencies");

void showAlert(const QString& message) {
  logger.log() << "Show alert:" << message;

  QMessageBox alert;
  alert.setText(message);
  alert.exec();
}

bool findInPath(const char* what) {
  char* path = getenv("PATH");
  Q_ASSERT(path);

  QStringList parts = QString(path).split(":");
  for (const QString& part : parts) {
    QDir pathDir(part);
    QFileInfo file(pathDir.filePath(what));
    if (file.exists()) {
      logger.log() << what << "found" << file.filePath();
      return true;
    }
  }

  return false;
}

bool checkDaemonVersion() {
  logger.log() << "Check Daemon Version";

  DBusClient* dbus = new DBusClient(nullptr);
  QDBusPendingCallWatcher* watcher = dbus->version();

  bool completed = false;
  bool value = false;
  QObject::connect(
      watcher, &QDBusPendingCallWatcher::finished,
      [completed = &completed, value = &value](QDBusPendingCallWatcher* call) {
        *completed = true;

        QDBusPendingReply<QString> reply = *call;
        if (reply.isError()) {
          logger.log() << "DBus message received - error";
          *value = false;
          return;
        }

        QString version = reply.argumentAt<0>();
        *value = version == PROTOCOL_VERSION;

        logger.log() << "DBus message received - daemon version:" << version
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

  if (!findInPath(WG_QUICK)) {
    showAlert("Unable to locate wg-quick");
    return false;
  }

  if (!checkDaemonVersion()) {
    showAlert("mozillavpn linuxdaemon needs to be updated or restarted.");
    return false;
  }

  return true;
}
