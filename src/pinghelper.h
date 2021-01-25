/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PINGHELPER_H
#define PINGHELPER_H

#include <QList>
#include <QObject>
#include <QThread>
#include <QTimer>

class PingSender;

class PingHelper final : public QObject {
 private:
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(PingHelper)

 public:
  PingHelper();
  ~PingHelper();

  void start(const QString& serverIpv4Gateway);

  void stop();

 signals:
  void pingSentAndReceived(qint64 msec);

 private:
  void nextPing();

  void pingReceived(PingSender* pingSender, qint64 msec);

 private:
  QString m_gateway;

  QTimer m_pingTimer;

  QList<PingSender*> m_pings;

  QThread m_pingThread;
};

#endif  // PINGHELPER_H
