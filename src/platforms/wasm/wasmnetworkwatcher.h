/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WASMNETWORKWATCHER_H
#define WASMNETWORKWATCHER_H

#include <QNetworkInformation>

#include "networkwatcherimpl.h"

class WasmNetworkWatcher final : public NetworkWatcherImpl {
  Q_OBJECT

 public:
  WasmNetworkWatcher(QObject* parent);
  ~WasmNetworkWatcher();

  void initialize() override;

  void start() override;
};

#endif  // WASMNETWORKWATCHER_H
