/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not needed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QQmlEngine>

#include "utils/loghandler.h"

struct MZLog {
  Q_GADGET
  QML_FOREIGN(LogHandler)
  QML_NAMED_ELEMENT(MZLog)
  QML_SINGLETON

 public:
  static LogHandler* create(QQmlEngine*, QJSEngine*) {
    return LogHandler::instance();
  }
};
