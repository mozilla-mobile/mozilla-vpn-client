/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "counter.h"
#include "glean/extern.h"

void CounterMetric::add(int amount) const { glean_counter_add(m_id, amount); }
