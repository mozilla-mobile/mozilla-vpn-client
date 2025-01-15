/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef STRING_H
#define STRING_H
#include <QObject>
#include <QString>

#include "basemetric.h"
#include "errortype.h"

class StringMetric final : public BaseMetric {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(StringMetric)

 public:
  explicit StringMetric(int aId);

  Q_INVOKABLE void set(QString value = "") const;

  // Test  only functions
  virtual QJsonValue testGetValue(const QString& pingName = "") const;
  virtual int32_t testGetNumRecordedErrors(ErrorType errorType) const;
};

#endif  // STRING_H
