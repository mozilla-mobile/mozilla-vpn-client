/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef STRING_H
#define STRING_H
#include <QObject>
#include <QString>

#include "errortype.h"

class StringMetric final {
  Q_GADGET

 public:
  // QML custom types require these three declarations.
  // See: https://doc.qt.io/qt-6/custom-types.html#creating-a-custom-type
  StringMetric() = default;
  StringMetric(const StringMetric&) = default;
  StringMetric& operator=(const StringMetric&) = default;

  explicit StringMetric(int aId);
  ~StringMetric() = default;

  Q_INVOKABLE void set(QString value = "") const;

  // Test  only functions

  Q_INVOKABLE QString testGetValue(const QString& pingName = "") const;
  Q_INVOKABLE int32_t testGetNumRecordedErrors(ErrorType errorType) const;

 private:
  int m_id;
};

#endif  // STRING_H
