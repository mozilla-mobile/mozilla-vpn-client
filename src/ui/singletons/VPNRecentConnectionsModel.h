/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNRECENTCONNECTIONSMODEL_H
#define VPNRECENTCONNECTIONSMODEL_H

#include <QQmlEngine>

#include "models/recentconnections.h"

struct VPNRecentConnectionsModel {
  Q_GADGET
  QML_FOREIGN(RecentConnections)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static RecentConnections* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine) {
    return RecentConnections::instance();
  }
};

#endif  // VPNRECENTCONNECTIONSMODEL_H
