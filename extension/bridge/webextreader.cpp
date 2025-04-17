/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "webextreader.h"

#include <QIODevice>

WebExtReader::WebExtReader(QIODevice* d, QObject* parent) : QObject(parent) {
  m_device = d;
}

void WebExtReader::readyRead() {
  while (!m_device->atEnd()) {
    if (m_length == 0) {
      m_device->startTransaction();
      char msgLength[sizeof(uint32_t)];
      qint64 rx = m_device->read(msgLength, sizeof(msgLength));
      if (rx < sizeof(quint32)) {
        m_device->rollbackTransaction();
        return;
      }
      m_length = *reinterpret_cast<uint32_t*>(msgLength);
      if (m_length == 0) {
        continue;
      }
      m_buffer.reserve(m_length);
      m_device->commitTransaction();
    }

    // Read some more data.
    size_t maxlen = m_length - m_buffer.length();
    QByteArray data = m_device->read(maxlen);
    Q_ASSERT(data.length() <= maxlen);
    m_buffer.append(data);
    if (m_buffer.length() < m_length) {
      return;
    }

    // If the buffer is full, emit the message and reset.
    emit messageReceived(m_buffer);
    m_buffer.clear();
    m_length = 0;
  }

  // If we get here, then there is no more data to be read.
  disconnect(m_connection);
  emit eofReceived();
}
