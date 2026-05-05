/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QQmlEngine>

#include "models/licensemodel.h"

struct MZLicenseModel {
  Q_GADGET
  QML_FOREIGN(LicenseModel)
  QML_NAMED_ELEMENT(MZLicenseModel)
  QML_SINGLETON

 public:
  static LicenseModel* create(QQmlEngine*, QJSEngine*) {
    return LicenseModel::instance();
  }
};
