/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nebula.h"

constexpr auto QRC_ROOT = ":/nebula";

void Nebula::Initialize(QQmlEngine* engine) {
  engine->addImportPath(QRC_ROOT);
}
