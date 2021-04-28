/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "constants.h"

#include <QDir>

namespace {

bool s_initialized = false;
bool s_inProduction = true;
}  // namespace

bool Constants::inProduction() {
  if (!s_initialized) {
    s_initialized = true;
    s_inProduction = !QDir::home().exists(".mozillavpn_in_staging.txt");
  }

  return s_inProduction;
}
