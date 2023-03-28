/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef UUID_H
#define UUID_H
#include <QObject>

#include "errortype.h"

class UuidMetric final {
  Q_GADGET

 public:
  // QML custom types require these three declarations.
  // See: https://doc.qt.io/qt-6/custom-types.html#creating-a-custom-type
  UuidMetric() = default;
  UuidMetric(const UuidMetric&) = default;
  UuidMetric& operator=(const UuidMetric&) = default;

  explicit UuidMetric(int aId);
  ~UuidMetric() = default;

  Q_INVOKABLE void set(const QString& uuid) const;
  Q_INVOKABLE QString generateAndSet() const;

  // Test  only functions
  Q_INVOKABLE QString testGetValue(const QString& pingName = "") const;
  Q_INVOKABLE int32_t testGetNumRecordedErrors(ErrorType errorType) const;

 private:
  int m_id;
};

#endif  // UUID_H
