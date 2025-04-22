/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "webexthandler.h"

#include <QJsonObject>
#include <QProcess>
#include <QString>
 
#include <unistd.h>

void WebExtHandler::proc_info(const QByteArray& msg) {
  Q_UNUSED(msg);
  
  // Gather the parent process details.
  pid_t parent = getppid();
  QString procexe = QString("/proc/%1/exe").arg(parent);
  char path[PATH_MAX];
  ssize_t pathlen = readlink(procexe.toLocal8Bit(), path, PATH_MAX);

  // Send the response.
  QJsonObject obj;
  obj["pid"] = QJsonValue(parent);
  if (pathlen > 0) {
    obj["exe"] = QString(QByteArray(path, pathlen));
  }
  writeJsonStdout(obj);
}

void WebExtHandler::start(const QByteArray& msg) {
  Q_UNUSED(msg);

  QProcess proc;
  proc.setStandardInputFile(QProcess::nullDevice());
  proc.setStandardOutputFile(QProcess::nullDevice());
  proc.setStandardErrorFile(QProcess::nullDevice());
  proc.setProgram("gtk-launch");
  proc.setArguments(QStringList() << "org.mozilla.vpn");

  QJsonObject obj;
  if (!proc.startDetached()) {
    qWarning() << "Failed to launch vpn client:" << proc.error();
    obj["error"] = "start_failed";
  } else {
    obj["status"] = "requested_start";
  }
  writeJsonStdout(obj);
}
