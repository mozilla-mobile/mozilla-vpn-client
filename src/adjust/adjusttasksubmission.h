/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADJUSTTASKSUBMISSION_H
#define ADJUSTTASKSUBMISSION_H

#include "task.h"

#include <QObject>

class AdjustTaskSubmission final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AdjustTaskSubmission)

 public:
  AdjustTaskSubmission(const QString& method, const QString& path,
                       const QList<QPair<QString, QString>>& headers,
                       const QString& queryParameters,
                       const QString& bodyParameters,
                       const QStringList& unknownParameters);

  ~AdjustTaskSubmission();

  void run() override;

 signals:
  void operationCompleted(const QByteArray& data, int statusCode);

 private:
  const QString m_method;
  const QString m_path;
  const QList<QPair<QString, QString>>& m_headers;
  const QString m_queryParameters;
  const QString m_bodyParameters;
  const QStringList m_unknownParameters;
};

#endif  // ADJUSTTASKSUBMISSION_H
