/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSDAEMON_H
#define WINDOWSDAEMON_H

#include "daemon/daemon.h"
#include "windowstunnelmonitor.h"

#include <QDateTime>

#define TUNNEL_SERVICE_NAME L"WireGuardTunnel$MozillaVPN"

class WindowsDaemon final : public Daemon {
  Q_DISABLE_COPY_MOVE(WindowsDaemon)

 public:
  WindowsDaemon();
  ~WindowsDaemon();

  bool activate(const Config& config) override;

  QByteArray status() override;

 private:
  bool run(Op op, const Config& config) override;

  bool supportServerSwitching(const Config& config) const override;

  bool switchServer(const Config& config) override;

  bool registerTunnelService(const QString& configFile);

 private:
  void monitorBackendFailure();

 private:
  QDateTime m_connectionDate;

  enum State {
    Active,
    Inactive,
  };

  State m_state = Inactive;

  WindowsTunnelMonitor m_tunnelMonitor;
};

#endif  // WINDOWSDAEMON_H
