/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOSNETWORKWATCHER_H
#define IOSNETWORKWATCHER_H

#include "networkwatcherimpl.h"
#include <Network/Network.h>

class IOSNetworkWatcher final : public NetworkWatcherImpl {
 public:
  IOSNetworkWatcher(QObject* parent);
  ~IOSNetworkWatcher();

  void initialize() override;
  NetworkWatcherImpl::TransportType getTransportType() override;

 private:
  NetworkWatcherImpl::TransportType mCurrentDefaultTransport =
      NetworkWatcherImpl::TransportType_Unknown;
  NetworkWatcherImpl::TransportType mCurrentVPNTransport =
      NetworkWatcherImpl::TransportType_Unknown;
  nw_path_monitor_t mNetworkMonitor = nil;
  nw_connection_t mObservableConnection = nil;

  NetworkWatcherImpl::TransportType toTransportType(nw_path_t path);
  void controllerStateChanged();
};

#endif  // IOSNETWORKWATCHER_H
