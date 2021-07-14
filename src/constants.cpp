/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "constants.h"
#include "settingsholder.h"

namespace {
bool s_inProduction = true;
}  // namespace

bool Constants::inProduction() { return s_inProduction; }

void Constants::setStaging() { s_inProduction = false; }
