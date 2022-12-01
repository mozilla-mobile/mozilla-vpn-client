/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean/glean.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger(LOG_MAIN, "Glean-noop");
}

VPNGlean::VPNGlean() { MVPN_COUNT_CTOR(VPNGlean); }

VPNGlean::~VPNGlean() { MVPN_COUNT_DTOR(VPNGlean); }

// static
void VPNGlean::initialize() { logger.debug() << "Initializing VPNGlean"; }

// static
void VPNGlean::setUploadEnabled(bool isTelemetryEnabled) {
  logger.debug() << "Changing VPNGlean upload status to" << isTelemetryEnabled;
}
