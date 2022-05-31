/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSNETWORKWATCHER_H
#define MACOSNETWORKWATCHER_H

#include "networkwatcherimpl.h"

#import <Network/Network.h>

class QString;

class MacOSNetworkWatcher final : public NetworkWatcherImpl {
 public:
  MacOSNetworkWatcher(QObject* parent);
  ~MacOSNetworkWatcher();

  void initialize() override;

  void start() override;

  void checkInterface();

  NetworkWatcherImpl::TransportType getTransportType() override;

  void controllerStateChanged();

 private:
  void* m_delegate = nullptr;
  NetworkWatcherImpl::TransportType mCurrentDefaultTransport =
      NetworkWatcherImpl::TransportType_Unknown;
  NetworkWatcherImpl::TransportType mVPNTunnelTransport =
      NetworkWatcherImpl::TransportType_Unknown;
  void* m_networkMonitor = nullptr;

  nw_connection_t m_observableConnection = nil;

  NetworkWatcherImpl::TransportType toTransportType(nw_path_t path);
};

#endif  // MACOSNETWORKWATCHER_H
