/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BOOLEAN_H
#define BOOLEAN_H
#include <QObject>

#include "errortype.h"

class BooleanMetric final {
  Q_GADGET

 public:
  // QML custom types require these three declarations.
  // See: https://doc.qt.io/qt-6/custom-types.html#creating-a-custom-type
  BooleanMetric() = default;
  BooleanMetric(const BooleanMetric&) = default;
  BooleanMetric& operator=(const BooleanMetric&) = default;

  explicit BooleanMetric(int aId);
  ~BooleanMetric() = default;

  Q_INVOKABLE void set(bool value = true) const;

  // Test  only functions
  Q_INVOKABLE bool testGetValue(const QString& pingName = "") const;
  Q_INVOKABLE int32_t testGetNumRecordedErrors(ErrorType errorType) const;

 private:
  int m_id;
};

#endif  // BOOLEAN_H
