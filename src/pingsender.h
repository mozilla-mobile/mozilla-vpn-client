/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PINGSENDER_H
#define PINGSENDER_H

#include <QElapsedTimer>
#include <QObject>

class PingSender : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(PingSender)

 public:
  PingSender(QObject* parent = nullptr) : QObject(parent) {}

  virtual void sendPing(const QString& destination, quint16 sequence) = 0;

  static quint16 inetChecksum(const void* data, size_t length);

 protected:
  // QProcess is not supported on iOS. Because of this we cannot use the `ping`
  // app as fallback on this platform.
#ifndef MVPN_IOS
  void genericSendPing(const QStringList& args, qint16 sequence);
#endif

 signals:
  void recvPing(quint16 sequence);
};

#endif  // PINGSENDER_H
