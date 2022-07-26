/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef UPLOADDATAGENERATOR_H
#define UPLOADDATAGENERATOR_H

#include <QIODevice>

class UploadDataGenerator : public QIODevice {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(UploadDataGenerator)

 public:
  explicit UploadDataGenerator(const qint64& totalSize);
  virtual ~UploadDataGenerator();

  qint64 size() const override {
    return m_totalSize;
  }

 protected:
  virtual qint64 readData(char* data, qint64 maxSize) override;
  virtual qint64 writeData(const char* data, qint64 maxSize) override;
  
 private:
  const qint64 m_totalSize;
};

#endif  // UPLOADDATAGENERATOR_H
