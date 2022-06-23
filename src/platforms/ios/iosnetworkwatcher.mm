/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "iosnetworkwatcher.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"

#import <Network/Network.h>

namespace {
Logger logger(LOG_IOS, "IOSNetworkWatcher");
dispatch_queue_t s_queue = dispatch_queue_create("VPNNetwork.queue", DISPATCH_QUEUE_SERIAL);
}

IOSNetworkWatcher::IOSNetworkWatcher(QObject* parent) : NetworkWatcherImpl(parent) {
  MVPN_COUNT_CTOR(IOSNetworkWatcher);
}

IOSNetworkWatcher::~IOSNetworkWatcher() {
  MVPN_COUNT_DTOR(IOSNetworkWatcher);
  if (m_networkMonitor != nil) {
    nw_path_monitor_cancel(m_networkMonitor);
    nw_release(m_networkMonitor);
  }
}

void IOSNetworkWatcher::initialize() {
  m_networkMonitor = nw_path_monitor_create();
  nw_path_monitor_set_queue(m_networkMonitor, s_queue);
  nw_path_monitor_set_update_handler(m_networkMonitor, ^(nw_path_t _Nonnull path) {
    m_currentDefaultTransport = toTransportType(path);
  });
  nw_path_monitor_start(m_networkMonitor);

  connect(MozillaVPN::instance()->controller(), &Controller::stateChanged, this,
          &IOSNetworkWatcher::controllerStateChanged);
}

NetworkWatcherImpl::TransportType IOSNetworkWatcher::getTransportType() {
  if (MozillaVPN::instance()->controller()->state() != Controller::StateOn) {
    // If we're not in stateON, the result from the Observer is fine, as
    // the default-route-transport  is not the vpn-tunnel
    return m_currentDefaultTransport;
  }
  if (m_observableConnection != nil) {
    return m_currentVPNTransport;
  }
  // If we don't have an open tunnel-observer, m_currentVPNTransport is probably wrong.
  return NetworkWatcherImpl::TransportType_Unknown;
}

NetworkWatcherImpl::TransportType IOSNetworkWatcher::toTransportType(nw_path_t path) {
  if (path == nil) {
    return NetworkWatcherImpl::TransportType_Unknown;
  }
  auto status = nw_path_get_status(path);
  if (status != nw_path_status_satisfied && status != nw_path_status_satisfiable) {
    // We're offline.
    return NetworkWatcherImpl::TransportType_None;
  }
  if (nw_path_uses_interface_type(path, nw_interface_type_wifi)) {
    return NetworkWatcherImpl::TransportType_WiFi;
  }
  if (nw_path_uses_interface_type(path, nw_interface_type_wired)) {
    return NetworkWatcherImpl::TransportType_Ethernet;
  }
  if (nw_path_uses_interface_type(path, nw_interface_type_cellular)) {
    return NetworkWatcherImpl::TransportType_Cellular;
  }
  if (nw_path_uses_interface_type(path, nw_interface_type_other)) {
    return NetworkWatcherImpl::TransportType_Other;
  }
  if (nw_path_uses_interface_type(path, nw_interface_type_loopback)) {
    return NetworkWatcherImpl::TransportType_Other;
  }
  return NetworkWatcherImpl::TransportType_Unknown;
}

void IOSNetworkWatcher::controllerStateChanged() {
  if (MozillaVPN::instance()->controller()->state() != Controller::StateOn) {
    if (m_observableConnection != nil) {
      nw_connection_cancel(m_observableConnection);
      nw_release(m_observableConnection);
      m_observableConnection = nil;
    }
    return;
  }
  // When connected the current default route is the VPN (ofc)
  // therefore the monitor will report transport::other.
  // We need to open up a theoretical socket to our entry-node, to find out
  // what connection the route is using
  auto vpn = MozillaVPN::instance();
  // When multihop is used, we need to connect to the entry server,
  // otherwise the exit server is the target
  auto key = vpn->entryServerPublicKey();
  auto serverlist = vpn->entryServers();
  auto index = serverlist.indexOf(key);
  // No such server
  if (index == -1) {
    return;
  }
  auto server = serverlist.at(index);
  auto str = server.ipv4AddrIn().toStdString();
  auto endpoint = nw_endpoint_create_host(str.c_str(), "80");
  auto params = nw_parameters_create_secure_udp(NW_PARAMETERS_DISABLE_PROTOCOL,
                                                NW_PARAMETERS_DEFAULT_CONFIGURATION);
  m_observableConnection = nw_connection_create(endpoint, params);
  nw_connection_set_queue(m_observableConnection, s_queue);
  nw_connection_set_path_changed_handler(m_observableConnection, ^(nw_path_t _Nonnull path) {
    m_currentVPNTransport = toTransportType(path);
  });
  nw_connection_start(m_observableConnection);
}
