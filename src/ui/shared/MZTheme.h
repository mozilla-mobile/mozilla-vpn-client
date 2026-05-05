/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QQmlEngine>

#include "theme.h"

struct MZTheme {
  Q_GADGET
  QML_FOREIGN(Theme)
  QML_NAMED_ELEMENT(MZTheme)
  QML_SINGLETON

 public:
  static Theme* create(QQmlEngine* engine, QJSEngine*) {
    Theme::instance()->initialize(engine);
    return Theme::instance();
  }
};
