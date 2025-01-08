/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef UUID_H
#define UUID_H
#include <QObject>

#include "basemetric.h"
#include "errortype.h"

class UuidMetric final : public BaseMetric {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(UuidMetric)

 public:
  explicit UuidMetric(int aId);
  ~UuidMetric() = default;

  Q_INVOKABLE void set(const QString& uuid) const;
  Q_INVOKABLE QString generateAndSet() const;

  // Test  only functions
  virtual QJsonValue testGetValue(const QString& pingName = "") const;
  virtual int32_t testGetNumRecordedErrors(ErrorType errorType) const;
};

#endif  // UUID_H
