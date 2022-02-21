/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONBENCHMARK_H
#define CONNECTIONBENCHMARK_H

#include "qmlengineholder.h"
#include "l18nstrings.h"

#include <QObject>
#include <QApplication>

class ConnectionBenchmark : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString id MEMBER m_id CONSTANT)
  Q_PROPERTY(QString displayName MEMBER m_displayName CONSTANT)
  Q_PROPERTY(quint64 result MEMBER m_result CONSTANT)
  Q_PROPERTY(bool isAvailable READ isAvailable NOTIFY isAvailableChanged)

 public:
  ConnectionBenchmark(const QString& id, const QString& displayName,
                      quint64 result, bool isAvailable);
  ~ConnectionBenchmark();

  bool isAvailable() const;

  QString id() const { return m_id; }
  QString displayName() const;
  quint64 result() const { return m_result; }

 signals:
  void isAvailableChanged();

 private:
  const QString m_id;
  const QString m_displayName;
  const quint64 m_result;
  bool m_isAvailable;
};

#endif  // CONNECTIONBENCHMARK_H