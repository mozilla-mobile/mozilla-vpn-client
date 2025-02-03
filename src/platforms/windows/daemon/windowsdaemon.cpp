/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsdaemon.h"

#include <Windows.h>
#include <qassert.h>

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLocalSocket>
#include <QNetworkInterface>
#include <QTextStream>
#include <QtGlobal>

#include "daemon/daemonerrors.h"
#include "dnsutilswindows.h"
#include "leakdetector.h"
#include "logger.h"
#include "platforms/windows/daemon/windowsfirewall.h"
#include "platforms/windows/daemon/windowssplittunnel.h"
#include "platforms/windows/windowscommons.h"
#include "windowsfirewall.h"

namespace {
Logger logger("WindowsDaemon");
}

WindowsDaemon::WindowsDaemon() : Daemon(nullptr) {
  MZ_COUNT_CTOR(WindowsDaemon);
  m_firewallManager = WindowsFirewall::create(this);
  Q_ASSERT(m_firewallManager != nullptr);

  m_wgutils = WireguardUtilsWindows::create(m_firewallManager, this);
  m_dnsutils = new DnsUtilsWindows(this);
  m_splitTunnelManager = WindowsSplitTunnel::create(m_firewallManager);

  connect(m_wgutils.get(), &WireguardUtilsWindows::backendFailure, this,
          &WindowsDaemon::monitorBackendFailure);
  connect(this, &WindowsDaemon::activationFailure,
          [this]() { m_firewallManager->disableKillSwitch(); });
}

WindowsDaemon::~WindowsDaemon() {
  MZ_COUNT_DTOR(WindowsDaemon);
  logger.debug() << "Daemon released";
}

bool WindowsDaemon::run(Op op, const InterfaceConfig& config) {
  if (!m_splitTunnelManager) {
    if (config.m_vpnDisabledApps.length() > 0) {
      // The Client has sent us a list of disabled apps, but we failed
      // to init the the split tunnel driver.
      // So let the client know this was not possible
      emit backendFailure(DaemonError::ERROR_SPLIT_TUNNEL_INIT_FAILURE);
    }
    return true;
  }

  if (op == Down) {
    m_splitTunnelManager->stop();
    return true;
  }
  if (config.m_vpnDisabledApps.length() > 0) {
    // Before creating the interface we need to check which adapter routes to
    // the entry server endpoint. This will be selected as the outgoing
    // interface for split-tunnelled traffic.
    QHostAddress entryServerAddr;
    if (config.m_hopType == InterfaceConfig::HopType::SingleHop) {
      entryServerAddr.setAddress(config.m_serverIpv4AddrIn);
    } else if (config.m_hopType == InterfaceConfig::HopType::MultiHopExit) {
      auto entry = m_connections.value(InterfaceConfig::HopType::MultiHopEntry);
      entryServerAddr.setAddress(entry.m_config.m_serverIpv4AddrIn);
    } else {
      return true;
    }

    int inetAdapterIndex = WindowsCommons::AdapterIndexTo(entryServerAddr);
    if (inetAdapterIndex < 0) {
      emit backendFailure(DaemonError::ERROR_SPLIT_TUNNEL_START_FAILURE);
    }

    if (!m_splitTunnelManager->start(inetAdapterIndex)) {
      emit backendFailure(DaemonError::ERROR_SPLIT_TUNNEL_START_FAILURE);
    };
    if (!m_splitTunnelManager->excludeApps(config.m_vpnDisabledApps)) {
      emit backendFailure(DaemonError::ERROR_SPLIT_TUNNEL_EXCLUDE_FAILURE);
    };
    // Now the driver should be running (State == 4)
    if (!m_splitTunnelManager->isRunning()) {
      emit backendFailure(DaemonError::ERROR_SPLIT_TUNNEL_START_FAILURE);
    }
    return true;
  }
  m_splitTunnelManager->stop();
  return true;
}

void WindowsDaemon::monitorBackendFailure() {
  logger.warning() << "Tunnel service is down";

  emit backendFailure();
  deactivate();
}
