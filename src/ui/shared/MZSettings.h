/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QQmlEngine>

#include "settingsholder.h"

struct MZSettings {
  Q_GADGET
  QML_FOREIGN(SettingsHolder)
  QML_NAMED_ELEMENT(MZSettings)
  QML_SINGLETON

 public:
  static SettingsHolder* create(QQmlEngine*, QJSEngine*) {
    return SettingsHolder::instance();
  }
};
