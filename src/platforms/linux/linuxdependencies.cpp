/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxdependencies.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

#include "dbusclient.h"
#include "logger.h"

namespace {

Logger logger("LinuxDependencies");

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
