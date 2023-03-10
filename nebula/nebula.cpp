/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nebula.h"

constexpr auto QRC_ROOT = "qrc:///nebula/";

void Nebula::Initialize(QQmlEngine* engine) {
#ifndef BUILD_QMAKE
  Q_INIT_RESOURCE(components);
  Q_INIT_RESOURCE(themes);
  Q_INIT_RESOURCE(nebula_resources);
  Q_INIT_RESOURCE(compatQt6);
  Q_INIT_RESOURCE(resourcesQt6);
  Q_INIT_RESOURCE(utils);
#endif

  engine->addImportPath(QRC_ROOT);
}
