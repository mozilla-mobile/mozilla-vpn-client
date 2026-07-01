/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dummyobfuscator.h"

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("DummyObfuscator");
}

DummyObfuscator::DummyObfuscator(const InterfaceConfig& config) {
  MZ_COUNT_CTOR(DummyObfuscator);
  Q_UNUSED(config);
}

bool DummyObfuscator::start() {
  logger.warning() << "Obfuscation is not supported on this platform";
  return false;
}

DummyObfuscator::~DummyObfuscator() { MZ_COUNT_DTOR(DummyObfuscator); }
