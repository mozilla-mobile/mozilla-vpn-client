/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NULLIODEVICE_H
#define NULLIODEVICE_H

#include <QIODevice>

class NullIODevice : public QIODevice {
 public:
  NullIODevice(QObject* parent = nullptr) : QIODevice(parent) {}

  virtual bool isSequential() const;

 protected:
  virtual qint64 readData(char* data, qint64 maxSize) override;
  virtual qint64 writeData(const char* data, qint64 maxSize) override;
};

#endif
