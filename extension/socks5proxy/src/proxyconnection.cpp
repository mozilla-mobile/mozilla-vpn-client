/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "proxyconnection.h"

ProxyConnection::ProxyConnection(QIODevice* socket) : QObject(socket) {}

void ProxyConnection::proxy(QIODevice* from, QIODevice* to,
                             quint64& watermark) {
  Q_ASSERT(from && to);

  for (;;) {
    qint64 available = from->bytesAvailable();
    if (available <= 0) {
      break;
    }

    qint64 capacity = MAX_CONNECTION_BUFFER - to->bytesToWrite();
    if (capacity <= 0) {
      break;
    }

    QByteArray data = from->read(qMin(available, capacity));
    if (data.length() == 0) {
      break;
    }
    qint64 sent = to->write(data);
    if (sent != data.length()) {
      qDebug() << "Truncated write. Sent" << sent << "of" << data.length();
      break;
    }
  }

  // Update buffer high watermark.
  qint64 queued = to->bytesToWrite();
  if (queued > watermark) {
    watermark = queued;
  }
}
