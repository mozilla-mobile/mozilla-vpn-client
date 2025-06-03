/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean/customdistribution.h"

#ifndef __wasm__
#  include "qtglean.h"
#endif

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QPair>

CustomDistributionMetric::CustomDistributionMetric(int id) : BaseMetric(id) {}

void CustomDistributionMetric::accumulate_single_sample(qint64 sample) const {
#ifndef __wasm__
  glean_custom_distribution_accumulate_sample(m_id, sample);
#else
  Q_UNUSED(sample);
#endif
}

int32_t CustomDistributionMetric::testGetNumRecordedErrors(
    ErrorType errorType) const {
#ifndef __wasm__
  return glean_custom_distribution_test_get_num_recorded_errors(m_id,
                                                                errorType);
#else
  Q_UNUSED(errorType);
  return 0;
#endif
}

QJsonValue CustomDistributionMetric::testGetValue(
    const QString& pingName) const {
#ifndef __wasm__
  auto value = QJsonDocument::fromJson(
      glean_custom_distribution_test_get_value(m_id, pingName.toUtf8()));

  return QJsonValue(value.object());
#else
  Q_UNUSED(pingName);
  return QJsonValue(QJsonObject());
#endif
}
