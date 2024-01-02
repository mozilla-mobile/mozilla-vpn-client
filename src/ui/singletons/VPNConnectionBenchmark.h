/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNCONNECTIONBENCHMARK_H
#define VPNCONNECTIONBENCHMARK_H

#include <QQmlEngine>

#include "connectionbenchmark/connectionbenchmark.h"
#include "mozillavpn.h"

struct VPNConnectionBenchmark {
  Q_GADGET
  QML_FOREIGN(ConnectionBenchmark)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static ConnectionBenchmark* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return MozillaVPN::instance()->connectionBenchmark();
  }
};

#endif  // VPNCONNECTIONBENCHMARK_H
