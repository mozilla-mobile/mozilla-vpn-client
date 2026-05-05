/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QQmlEngine>

#include "localizer.h"

struct MZLocalizer {
  Q_GADGET
  QML_FOREIGN(Localizer)
  QML_NAMED_ELEMENT(MZLocalizer)
  QML_SINGLETON

 public:
  static Localizer* create(QQmlEngine*, QJSEngine*) {
    return Localizer::instance();
  }
};
