/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "webextreader.h"

#include <QIODevice>
#include <QFileDevice>
#ifdef Q_OS_WIN
# include <QWinEventNotifier>
#else
# include <QSocketNotifier>
#endif

WebExtReader::WebExtReader(QIODevice* d, QObject* parent) : QObject(parent) {
  m_device = d;

  QFileDevice* fdev = qobject_cast<QFileDevice*>(d);
  if (fdev == nullptr) {
    m_connection = connect(m_device, &QIODevice::readyRead, this,
                           &WebExtReader::readyRead);
  } else {
#ifdef Q_OS_WIN
    m_connection = connect(new QWinEventNotifier(fdev->handle(), this),
                           &QWinEventNotifier::activated, this,
                           &WebExtReader::readyRead);
#else
    auto n = new QSocketNotifier(fdev->handle(), QSocketNotifier::Read, this);
    m_connection = connect(n, &QSocketNotifier::activated, this,
                           &WebExtReader::readyRead);
#endif
  }
}

void WebExtReader::readyRead() {
  while (!m_device->atEnd()) {
    // We are waiting on the message length.
    if (m_length == 0) {
      m_device->startTransaction();
      qint64 rx = m_device->read(reinterpret_cast<char*>(&m_length), sizeof(quint32));
      if (rx < sizeof(quint32)) {
        m_device->rollbackTransaction();
        return;
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
