/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QQmlEngine>

#include "env.h"

struct MZEnv {
  Q_GADGET
  QML_FOREIGN(Env)
  QML_NAMED_ELEMENT(MZEnv)
  QML_SINGLETON

 public:
  static Env* create(QQmlEngine*, QJSEngine*) { return Env::instance(); }
};
