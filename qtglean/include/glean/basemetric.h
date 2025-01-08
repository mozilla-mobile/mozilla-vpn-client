/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BASEMETRIC_H
#define BASEMETRIC_H
#include <QObject>

#include "errortype.h"

class BaseMetric : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(BaseMetric)

 public:
  explicit BaseMetric(int aId) : m_id(aId) {};

  // Test  only functions
  virtual QJsonValue testGetValue(const QString& pingName = "") const = 0;
  virtual int32_t testGetNumRecordedErrors(ErrorType errorType) const = 0;

 protected:
  const int m_id;
};

#endif  // BASEMETRIC_H
