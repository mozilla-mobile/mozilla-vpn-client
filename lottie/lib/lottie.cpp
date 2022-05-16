/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "lottie.h"
#include "lottieprivate.h"

constexpr auto QRC_ROOT = "qrc:///lottie/";

void Lottie::initialize(QQmlEngine* engine, const QString& userAgent) {
  Q_INIT_RESOURCE(lottie);
  engine->addImportPath(QRC_ROOT);
  LottiePrivate::initialize(engine, userAgent);
}
