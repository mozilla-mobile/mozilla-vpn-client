/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNRECOMMENDEDLOCATIONMODEL_H
#define VPNRECOMMENDEDLOCATIONMODEL_H

#include <QQmlEngine>

#include "models/recommendedlocationmodel.h"

struct VPNRecommendedLocationModel {
  Q_GADGET
  QML_FOREIGN(RecommendedLocationModel)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static RecommendedLocationModel* create(QQmlEngine* qmlEngine,
                                          QJSEngine* jsEngine) {
    return RecommendedLocationModel::instance();
  }
};

#endif  // VPNRECOMMENDEDLOCATIONMODEL_H
