/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDNETWORKWATCHER_H
#define ANDROIDNETWORKWATCHER_H

#include "networkwatcherimpl.h"

class AndroidNetworkWatcher final : public NetworkWatcherImpl {
 public:
  AndroidNetworkWatcher(QObject* parent);
  ~AndroidNetworkWatcher();

  void initialize() override;

  QNetworkInformation::Reachability getReachability() override;
};

#endif  // ANDROIDNETWORKWATCHER_H
