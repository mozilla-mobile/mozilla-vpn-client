/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nebula.h"

constexpr auto QRC_ROOT = "qrc:///nebula/";

void Nebula::Initialize(QQmlEngine* engine) {
  qputenv("QML_XHR_ALLOW_FILE_READ", QByteArray("1"));
  engine->addImportPath(QRC_ROOT);
}
