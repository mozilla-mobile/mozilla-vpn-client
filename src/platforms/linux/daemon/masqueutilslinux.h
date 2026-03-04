/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MASQUEUTILSLINUX_H
#define MASQUEUTILSLINUX_H

#include <QHostAddress>
#include <QObject>
#include <QProcess>
#include <QSocketNotifier>
#include <QStringList>

#include "daemon/masqueutils.h"
#include "linuxfirewall.h"

class MasqueUtilsLinux final : public MasqueUtils {
  Q_OBJECT

 public:
  MasqueUtilsLinux(QObject* parent);
  ~MasqueUtilsLinux();
  bool interfaceExists() override;
  bool addInterface(const InterfaceConfig& config) override;
  bool deleteInterface() override;

 private slots:
  void daemonStdoutReady();
  void daemonStderrReady();
  void daemonErrorOccurred(QProcess::ProcessError error);
  void daemonFinished(int exitCode, QProcess::ExitStatus exitStatus);

 private:
  LinuxFirewall m_firewall;
  QProcess m_daemonProcess;
};

#endif  // MASQUEUTILSLINUX_H
