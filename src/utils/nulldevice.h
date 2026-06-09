/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NULLDEVICE_H
#define NULLDEVICE_H

#include <QIODevice>

class NullDevice : public QIODevice {
 public:
  NullDevice(QObject* parent = nullptr) : QIODevice(parent) {}

  virtual bool isSequential() const;

 protected:
  virtual qint64 readData(char* data, qint64 maxSize) override;
  virtual qint64 writeData(const char* data, qint64 maxSize) override;
};

#endif
