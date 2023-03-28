/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DATETIME_H
#define DATETIME_H

#include <QDateTime>
#include <QObject>
#include <QString>

#include "errortype.h"

// NOTE: While most of datetime is implemented, one piece is not yet:
// - The ability to set an arbitrary datestamp
// More details: https://mozilla-hub.atlassian.net/browse/VPN-4173

class DatetimeMetric final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DatetimeMetric)

 public:
  explicit DatetimeMetric(int aId);

  Q_INVOKABLE void set() const;

  // Test  only functions

  Q_INVOKABLE QDateTime testGetValue(const QString& pingName = "") const;
  Q_INVOKABLE QString testGetValueAsString(const QString& pingName = "") const;
  Q_INVOKABLE int32_t testGetNumRecordedErrors(ErrorType errorType) const;

 private:
  const int m_id;
};

#endif  // DATETIME_H
