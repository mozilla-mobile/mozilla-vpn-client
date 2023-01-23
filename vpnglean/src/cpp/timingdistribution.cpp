/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean/timingdistribution.h"

#if not(defined(__wasm__) || defined(BUILD_QMAKE))
#  include "vpnglean.h"
#endif

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QPair>

TimingDistributionMetric::TimingDistributionMetric(int id) : m_id(id) {}

int TimingDistributionMetric::start() const {
#if not(defined(__wasm__) || defined(BUILD_QMAKE))
  return glean_timing_distribution_start(m_id);
#else
  return 0;
#endif
}

void TimingDistributionMetric::stopAndAccumulate(int timerId) const {
#if not(defined(__wasm__) || defined(BUILD_QMAKE))
  glean_timing_distribution_stop_and_accumulate(m_id, timerId);
#endif
}

void TimingDistributionMetric::cancel(int timerId) const {
#if not(defined(__wasm__) || defined(BUILD_QMAKE))
  glean_timing_distribution_cancel(m_id, timerId);
#endif
}

int32_t TimingDistributionMetric::testGetNumRecordedErrors(
    ErrorType errorType) const {
#if not(defined(__wasm__) || defined(BUILD_QMAKE))
  return glean_timing_distribution_test_get_num_recorded_errors(m_id,
                                                                errorType);
#else
  return 0;
#endif
}

DistributionData TimingDistributionMetric::testGetValue(
    const QString& pingName) const {
#if not(defined(__wasm__) || defined(BUILD_QMAKE))
  auto value = QJsonDocument::fromJson(
      glean_timing_distribution_test_get_value(m_id, pingName.toLocal8Bit()));

  DistributionData result;
  if (!value.isEmpty()) {
    result.sum = value["sum"].toInt();
    result.count = value["count"].toInt();

    QJsonObject values = value["values"].toObject();
    foreach (const QString& key, values.keys()) {
      result.values.insert(key.toInt(), values.take(key).toInt());
    }
  }

  return result;
#else
  return DistributionData();
#endif
}
