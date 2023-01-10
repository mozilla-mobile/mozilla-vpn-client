/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gleandeprecated.h"

#include <QCoreApplication>

#include "leakdetector.h"

GleanDeprecated::GleanDeprecated(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(GleanDeprecated);
}

GleanDeprecated::~GleanDeprecated() { MZ_COUNT_DTOR(GleanDeprecated); }

// static
GleanDeprecated* GleanDeprecated::instance() {
  static GleanDeprecated* instance = nullptr;
  if (!instance) {
    instance = new GleanDeprecated(qApp);
  }
  return instance;
}
