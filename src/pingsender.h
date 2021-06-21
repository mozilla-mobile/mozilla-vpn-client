/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PINGSENDER_H
#define PINGSENDER_H

#include <QElapsedTimer>
#include <QObject>

class PingSender final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(PingSender)

 public:
  PingSender(QObject* parent);
  ~PingSender();

  void send(const QString& destination, const QString& source);

 signals:
  void completed(PingSender* pingSender, qint64 msec);

  // internal only
  void sendPing(const QString& destination, const QString& source);

 private slots:
  void pingFailed();
  void pingSucceeded();

 private:
  QElapsedTimer m_time;
};

#endif  // PINGSENDER_H
