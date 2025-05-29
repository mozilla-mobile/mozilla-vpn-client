/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean/memorydistribution.h"

#ifndef __wasm__
#  include "bindings/qtglean.h"
#endif

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QPair>

// !!!IMPORTANT!!!
// All work for memory_distribution metric type was done as part of this
// PR: https://github.com/mozilla-mobile/mozilla-vpn-client/pull/9318
// After it was written, it was decided to use a custom_distribution
// data type instead.
// This memory distribution work has NOT been tested, though the code has
// stayed in so that this work is not lost, in the event we want to use
// this data type in the future.

MemoryDistributionMetric::MemoryDistributionMetric(int id) : BaseMetric(id) {}

void MemoryDistributionMetric::accumulate(qint64 sample) const {
#ifndef __wasm__
  glean_memory_distribution_accumulate(m_id, sample);
#else
  Q_UNUSED(sample);
#endif
}

int32_t MemoryDistributionMetric::testGetNumRecordedErrors(
    ErrorType errorType) const {
#ifndef __wasm__
  return glean_memory_distribution_test_get_num_recorded_errors(m_id,
                                                                errorType);
#else
  Q_UNUSED(errorType);
  return 0;
#endif
}

QJsonValue MemoryDistributionMetric::testGetValue(
    const QString& pingName) const {
#ifndef __wasm__
  auto value = QJsonDocument::fromJson(
      glean_memory_distribution_test_get_value(m_id, pingName.toUtf8()));

  return QJsonValue(value.object());
#else
  Q_UNUSED(pingName);
  return QJsonValue(QJsonObject());
#endif
}
