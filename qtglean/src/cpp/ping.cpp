/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean/ping.h"
#ifndef __wasm__
#  include "qtglean.h"
#endif

Ping::Ping(int aId) : m_id(aId) {}

void Ping::submit(const QString& reason = "") const {
#ifndef __wasm__
  glean_submit_ping_by_id(m_id, reason.toUtf8());
#endif
}
