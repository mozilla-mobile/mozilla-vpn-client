/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef EVENT_METRIC_H
#define EVENT_METRIC_H

#include "glean/extern.h"

template <class T>
class EventMetric final {
 public:
  constexpr explicit EventMetric(int aId) : m_id(aId) {}

  // Coudn't figure out how to have these declaredi n a cpp file and still have
  // access to T.
  void record() const { glean_event_record_no_extra(m_id); }

  void record(T extras) const {
    auto ffiExtras = extras.ToFfiExtra();
    glean_event_record(m_id, ffiExtras.keys, ffiExtras.values, ffiExtras.count);
  }

 private:
  int m_id;
};

#endif  // EVENT_METRIC_H
