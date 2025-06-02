/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean/timingdistribution.h"

#ifndef __wasm__
#  include "qtglean.h"
#endif

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QPair>

TimingDistributionMetric::TimingDistributionMetric(int id) : BaseMetric(id) {}

qint64 TimingDistributionMetric::start() const {
#ifndef __wasm__
  return glean_timing_distribution_start(m_id);
#else
  return 0;
#endif
}

void TimingDistributionMetric::stopAndAccumulate(qint64 timerId) const {
#ifndef __wasm__
  glean_timing_distribution_stop_and_accumulate(m_id, timerId);
#else
  Q_UNUSED(timerId);
#endif
}

void TimingDistributionMetric::cancel(qint64 timerId) const {
#ifndef __wasm__
  glean_timing_distribution_cancel(m_id, timerId);
#else
  Q_UNUSED(timerId);
#endif
}

int32_t TimingDistributionMetric::testGetNumRecordedErrors(
    ErrorType errorType) const {
#ifndef __wasm__
  return glean_timing_distribution_test_get_num_recorded_errors(m_id,
                                                                errorType);
#else
  Q_UNUSED(errorType);
  return 0;
#endif
}

QJsonValue TimingDistributionMetric::testGetValue(
    const QString& pingName) const {
#ifndef __wasm__
  auto value = QJsonDocument::fromJson(
      glean_timing_distribution_test_get_value(m_id, pingName.toUtf8()));

  return QJsonValue(value.object());
#else
  Q_UNUSED(pingName);
  return QJsonValue(QJsonObject());
#endif
}
