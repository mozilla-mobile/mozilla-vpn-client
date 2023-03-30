/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef STRING_H
#define STRING_H
#include <QObject>
#include <QString>

#include "errortype.h"

class StringMetric final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(StringMetric)

 public:
  explicit StringMetric(int aId);

  Q_INVOKABLE void set(QString value = "") const;

  // Test  only functions

  Q_INVOKABLE QString testGetValue(const QString& pingName = "") const;
  Q_INVOKABLE int32_t testGetNumRecordedErrors(ErrorType errorType) const;

 private:
  const int m_id;
};

#endif  // STRING_H
