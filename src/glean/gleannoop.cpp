/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean/glean.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger(LOG_MAIN, "Glean-noop");
}

Glean::Glean() { MVPN_COUNT_CTOR(Glean); }

Glean::~Glean() { MVPN_COUNT_DTOR(Glean); }

// static
void Glean::initialize() { logger.debug() << "Initializing Glean"; }

// static
void Glean::setUploadEnabled(bool isTelemetryEnabled) {
  logger.debug() << "Changing Glean upload status to" << isTelemetryEnabled;
}
