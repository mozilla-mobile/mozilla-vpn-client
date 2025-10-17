/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WEBEXTREADER_H
#define WEBEXTREADER_H

#include <QByteArray>
#include <QObject>

class QIODevice;

class WebExtReader : public QObject {
  Q_OBJECT

 public:
  WebExtReader(QIODevice* d, QObject* parent = nullptr);

  void readyRead();

 signals:
  void messageReceived(const QByteArray& message);

 private:
  QIODevice* m_device;
  QByteArray m_buffer;
  quint32 m_length = 0;
};

#endif  // WEBEXTREADER_H
