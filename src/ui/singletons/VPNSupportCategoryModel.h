/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNSUPPORTCATEGORYMODEL_H
#define VPNSUPPORTCATEGORYMODEL_H

#include <QQmlEngine>

#include "models/supportcategorymodel.h"
#include "mozillavpn.h"

struct VPNSupportCategoryModel {
  Q_GADGET
  QML_FOREIGN(SupportCategoryModel)
  QML_ELEMENT
  QML_SINGLETON

 public:
  static SupportCategoryModel* create(QQmlEngine* qmlEngine,
                                      QJSEngine* jsEngine) {
    return MozillaVPN::instance()->supportCategoryModel();
  }
};

#endif  // VPNSUPPORTCATEGORYMODEL_H
