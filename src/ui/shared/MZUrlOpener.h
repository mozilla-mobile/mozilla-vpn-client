/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QQmlEngine>

#include "urlopener.h"

struct MZUrlOpener {
  Q_GADGET
  QML_FOREIGN(UrlOpener)
  QML_NAMED_ELEMENT(MZUrlOpener)
  QML_SINGLETON

 public:
  static UrlOpener* create(QQmlEngine*, QJSEngine*) {
    return UrlOpener::instance();
  }
};
