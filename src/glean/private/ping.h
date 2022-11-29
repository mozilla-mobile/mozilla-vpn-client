/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PING_H
#define PING_H

#include "vpnglean.h"

class Ping final {
 public:
  constexpr explicit Ping(int aId) : m_id(aId) {}

  void submit() const { glean_submit_ping_by_id(m_id); };

 private:
  int m_id;
};

#endif  // PING_H
