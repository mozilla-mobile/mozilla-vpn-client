/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ping.h"
#include "glean/extern.h"

void Ping::submit() const { glean_submit_ping_by_id(m_id); }