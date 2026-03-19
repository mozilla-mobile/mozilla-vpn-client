/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MASQUETUNNELLINUX_H
#define MASQUETUNNELLINUX_H

#include <QHostAddress>
#include <QObject>
#include <QProcess>
#include <QSocketNotifier>
#include <QStringList>

#include "daemon/protocols/masque.h"
#include "platforms/linux/daemon/linuxfirewall.h"

class MasqueTunnelLinux final : public MasqueTunnel {
  Q_OBJECT

 public:
  MasqueTunnelLinux(QObject* parent);
  ~MasqueTunnelLinux();
  QJsonObject getStatus() const override;
  bool interfaceExists();
  bool addInterface(const InterfaceConfig& config) override;
  bool deleteInterface() override;
  void resetApp(const QString& app) override{};
  void excludeApp(const QString& app) override{};
  void resetAllApps() override{};
  bool supportSplitTunnel() override { return false; };
  bool waitForInterfaceReady();

 private slots:
  void daemonStdoutReady();
  void daemonStderrReady();
  void daemonErrorOccurred(QProcess::ProcessError error);
  void daemonFinished(int exitCode, QProcess::ExitStatus exitStatus);

 private:
  LinuxFirewall m_firewall;
  QProcess m_daemonProcess;
};

#endif  // MASQUETUNNELLINUX_H
