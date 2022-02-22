/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONBENCHMARKITEM_H
#define CONNECTIONBENCHMARKITEM_H

#include "qmlengineholder.h"
#include "l18nstrings.h"

#include <QObject>
#include <QApplication>

class ConnectionBenchmarkItem : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString id MEMBER m_id CONSTANT)
  Q_PROPERTY(QString displayName MEMBER m_displayName CONSTANT)
  Q_PROPERTY(QString icon MEMBER m_icon CONSTANT)
  Q_PROPERTY(quint64 result MEMBER m_result CONSTANT)

 public:
  ConnectionBenchmarkItem(const QString& id, const QString& displayName,
                          const QString& icon, quint64 result);
  ~ConnectionBenchmarkItem();

  bool isAvailable() const;

  QString id() const { return m_id; }
  QString displayName() const;
  QString icon() const { return m_icon; }
  quint64 result() const { return m_result; }

 private:
  const QString m_id;
  const QString m_displayName;
  const QString m_icon;
  const quint64 m_result;
};

#endif  // CONNECTIONBENCHMARKITEM_H