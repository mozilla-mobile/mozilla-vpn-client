
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

class DatetimeMetric final {
  Q_GADGET

 public:
  // QML custom types require these three declarations.
  // See: https://doc.qt.io/qt-6/custom-types.html#creating-a-custom-type
  DatetimeMetric() = default;
  DatetimeMetric(const DatetimeMetric&) = default;
  DatetimeMetric& operator=(const DatetimeMetric&) = default;

  explicit DatetimeMetric(int aId);
  ~DatetimeMetric() = default;

  Q_INVOKABLE void set() const;

  // Test  only functions

  Q_INVOKABLE QDateTime testGetValue(const QString& pingName = "") const;
  Q_INVOKABLE QString testGetValueAsString(const QString& pingName = "") const;
  Q_INVOKABLE int32_t testGetNumRecordedErrors(ErrorType errorType) const;

 private:
  int m_id;
};

#endif  // DATETIME_H
