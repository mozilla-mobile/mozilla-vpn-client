/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOSNETWORKWATCHER_H
#define IOSNETWORKWATCHER_H

#include "networkwatcherimpl.h"

class IOSNetworkWatcher final : public NetworkWatcherImpl {
 public:
  IOSNetworkWatcher(QObject* parent);
  ~IOSNetworkWatcher();

  void initialize() override;
  NetworkWatcherImpl::TransportType getTransportType() override;

 private:
  NetworkWatcherImpl::TransportType mCurrentTransport =
      NetworkWatcherImpl::TransportType_Unknown;
  void* m_networkMonitor = nullptr;
};

#endif  // IOSNETWORKWATCHER_H
