/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDTUNNELLINUX_H
#define WIREGUARDTUNNELLINUX_H

#include <QHostAddress>
#include <QObject>
#include <QProcess>
#include <QSocketNotifier>
#include <QStringList>

#include "daemon/protocols/wireguard.h"
#include "platforms/linux/daemon/iputilslinux.h"
#include "platforms/linux/daemon/linuxfirewall.h"
#include "platforms/linux/daemon/wireguardutilslinux.h"

class WireGuardTunnelLinux final : public WireGuardTunnel {
  Q_OBJECT

 public:
  WireGuardTunnelLinux(QObject* parent);
  ~WireGuardTunnelLinux();
  bool supportSplitTunnel() override { return true; };
  void resetApp(const QString& app) override { m_wgutils->resetCgroup(app); };
  void excludeApp(const QString& app) override {
    m_wgutils->excludeCgroup(app);
  };
  void resetAllApps() override { m_wgutils->resetAllCgroups(); };
  bool supportIPUtils() const override { return true; };

 protected:
  WireguardUtils* wgutils() const override { return m_wgutils; };
  IPUtils* iputils() override;

 private:
  WireguardUtilsLinux* m_wgutils = nullptr;
  IPUtilsLinux* m_iputils = nullptr;
};

#endif  // WIREGUARDTUNNELLINUX_H
