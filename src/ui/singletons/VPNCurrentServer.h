/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNCURRENTSERVER_H
#define VPNCURRENTSERVER_H

#include <QQmlEngine>

#include "models/serverdata.h"
#include "mozillavpn.h"

struct VPNCurrentServer {
  Q_GADGET
  QML_FOREIGN(ServerData)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static ServerData* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return MozillaVPN::instance()->serverData();
  }
};

#endif  // VPNCURRENTSERVER_H
