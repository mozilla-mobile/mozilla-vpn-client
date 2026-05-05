/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QQmlEngine>

#include "frontend/navigationbarmodel.h"

struct MZNavigationBarModel {
  Q_GADGET
  QML_FOREIGN(NavigationBarModel)
  QML_NAMED_ELEMENT(MZNavigationBarModel)
  QML_SINGLETON

 public:
  static NavigationBarModel* create(QQmlEngine*, QJSEngine*) {
    return NavigationBarModel::instance();
  }
};
