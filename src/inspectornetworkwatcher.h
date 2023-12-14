/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTORNETWORKWATCHER_H
#define INSPECTORNETWORKWATCHER_H

#include "networkwatcherimpl.h"

class InspectorNetworkWatcher final : public NetworkWatcherImpl {
 public:
  InspectorNetworkWatcher(QObject* parent);
  ~InspectorNetworkWatcher();

  void initialize() override;

  NetworkWatcherImpl::TransportType getTransportType() override {
    return TransportType_None;
  };
};

#endif  // INSPECTORNETWORKWATCHER_H
