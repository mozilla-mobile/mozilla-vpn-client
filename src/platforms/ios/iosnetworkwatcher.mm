/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "iosnetworkwatcher.h"
#include "leakdetector.h"
#include "logger.h"

#import <Network/Network.h>

namespace {
Logger logger(LOG_IOS, "IOSNetworkWatcher");
}

IOSNetworkWatcher::IOSNetworkWatcher(QObject* parent) : NetworkWatcherImpl(parent) {
  MVPN_COUNT_CTOR(IOSNetworkWatcher);
}

IOSNetworkWatcher::~IOSNetworkWatcher() {
  MVPN_COUNT_DTOR(IOSNetworkWatcher);
  if (m_networkMonitor) {
    nw_path_monitor_t mon = *((nw_path_monitor_t*)m_networkMonitor);
    nw_path_monitor_cancel(mon);
  }
}

void IOSNetworkWatcher::initialize() {
  dispatch_queue_t agentQueue = dispatch_queue_create("VPNNetwork.queue", DISPATCH_QUEUE_SERIAL);
  auto mon = nw_path_monitor_create();
  nw_path_monitor_set_queue(mon, agentQueue);
  nw_path_monitor_set_update_handler(mon, ^(nw_path_t _Nonnull path) {
    if (nw_path_get_status(path) != nw_path_status_satisfied &&
        nw_path_get_status(path) != nw_path_status_satisfiable) {
      // We're offline.
      this->mCurrentTransport = NetworkWatcherImpl::TransportType_None;
    }
    if (nw_path_uses_interface_type(path, nw_interface_type_wifi)) {
      this->mCurrentTransport = NetworkWatcherImpl::TransportType_WiFi;
      return;
    }
    if (nw_path_uses_interface_type(path, nw_interface_type_wired)) {
      this->mCurrentTransport = NetworkWatcherImpl::TransportType_Ethernet;
      return;
    }
    if (nw_path_uses_interface_type(path, nw_interface_type_cellular)) {
      this->mCurrentTransport = NetworkWatcherImpl::TransportType_Cellular;
      return;
    }
    if (nw_path_uses_interface_type(path, nw_interface_type_other)) {
      this->mCurrentTransport = NetworkWatcherImpl::TransportType_Other;
      return;
    }
    if (nw_path_uses_interface_type(path, nw_interface_type_loopback)) {
      this->mCurrentTransport = NetworkWatcherImpl::TransportType_Other;
      return;
    }
    this->mCurrentTransport = NetworkWatcherImpl::TransportType_Unknown;
  });
  nw_path_monitor_start(mon);
  m_networkMonitor = &mon;
}

NetworkWatcherImpl::TransportType IOSNetworkWatcher::getTransportType() {
  return mCurrentTransport;
}
