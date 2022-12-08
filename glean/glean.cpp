/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean.h"

#include <QQmlEngine>

constexpr auto QRC_ROOT = "qrc:///glean/";

void Glean::Initialize(QQmlEngine* engine) {
#ifndef BUILD_QMAKE
  Q_INIT_RESOURCE(glean);
  Q_INIT_RESOURCE(telemetry);
#endif
  engine->addImportPath(QRC_ROOT);
}
