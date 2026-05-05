/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QQmlEngine>

#include "addons/manager/addonmanager.h"

struct MZAddonManager {
  Q_GADGET
  QML_FOREIGN(AddonManager)
  QML_NAMED_ELEMENT(MZAddonManager)
  QML_SINGLETON

 public:
  static AddonManager* create(QQmlEngine*, QJSEngine*) {
    return AddonManager::instance();
  }
};
