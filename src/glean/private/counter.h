/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COUNTER_METRIC_H
#define COUNTER_METRIC_H

class CounterMetric final {
 public:
  constexpr explicit CounterMetric(int aId) : m_id(aId) {}

  void add(int amount = 1) const;

 private:
  int m_id;
};

#endif  // COUNTER_METRIC_H
