/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSDAEMON_H
#define WINDOWSDAEMON_H

#include "daemon/daemon.h"
#include "windowstunnelservice.h"
#include "wireguardutilswindows.h"

#define TUNNEL_SERVICE_NAME L"WireGuardTunnel$MozillaVPN"

class WindowsDaemon final : public Daemon {
  Q_DISABLE_COPY_MOVE(WindowsDaemon)

 public:
  WindowsDaemon();
  ~WindowsDaemon();

  QByteArray getStatus() override;

 protected:
  bool supportWGUtils() const override { return true; }
  WireguardUtils* wgutils() override { return m_wgutils; }

 private:
  bool supportServerSwitching(const InterfaceConfig& config) const override;

  bool switchServer(const InterfaceConfig& config) override;


 private:
  void monitorBackendFailure();

 private:
  enum State {
    Active,
    Inactive,
  };

  State m_state = Inactive;

  WindowsTunnelService m_tunnel;
  WireguardUtilsWindows* m_wgutils = nullptr;
};

#endif  // WINDOWSDAEMON_H
