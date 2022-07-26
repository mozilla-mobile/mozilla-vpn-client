/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "uploaddatagenerator.h"
#include "leakdetector.h"
#include "logger.h"

#include <QBuffer>
#include <QRandomGenerator>

constexpr int32_t MAX_BUFFER_SIZE = 4096;

namespace {
Logger logger(LOG_MAIN, "UploadDataGenerator");
}

UploadDataGenerator::UploadDataGenerator(const qint64& totalSize)
    : m_totalSize(totalSize) {
  MVPN_COUNT_CTOR(UploadDataGenerator);
}

UploadDataGenerator::~UploadDataGenerator() {
  MVPN_COUNT_DTOR(UploadDataGenerator);
}

qint64 UploadDataGenerator::readData(char* data, qint64 maxSize) {
#ifdef MVPN_DEBUG
  logger.debug() << "Read data" << maxSize;
#else
  Q_UNUSED(maxSize);
#endif

  // TODO: Allocate char buffer in constructor with fixed size
  QByteArray dataBuffer;
  int bitCounter = this->pos();
  while (dataBuffer.length() < MAX_BUFFER_SIZE && bitCounter < m_totalSize) {
    dataBuffer.append(QRandomGenerator::global()->generate() % 256);
    bitCounter += 1;
  }

  memcpy(data, dataBuffer.data(), dataBuffer.size());
  return dataBuffer.size();
};

qint64 UploadDataGenerator::writeData(const char* data, qint64 maxSize) {
  logger.debug() << "Write data";
  Q_UNUSED(data);
  Q_UNUSED(maxSize);

  return -1;
};