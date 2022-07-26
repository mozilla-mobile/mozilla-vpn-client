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

  int bitCounter = 0;
  while (bitCounter < MAX_BUFFER_SIZE && bitCounter < totalSize) {
    m_dataBuffer.append(QRandomGenerator::global()->generate() % 256);
    bitCounter++;
  }
}

UploadDataGenerator::~UploadDataGenerator() {
  MVPN_COUNT_DTOR(UploadDataGenerator);
}

qint64 UploadDataGenerator::readData(char* data, qint64 maxSize) {
  qint64 maxBufferSize = qMin(MAX_BUFFER_SIZE, maxSize);
  qint64 maxReadSize = qMin(m_totalSize - this->pos(), maxBufferSize);

#ifdef MVPN_DEBUG
  logger.debug() << "Read data" << maxReadSize;
#endif

  if (maxReadSize == 0) {
    return 0;
  } else if (maxReadSize < 0) {
    return -1;
  }

  memcpy(data, m_dataBuffer.constData(), maxReadSize);
  return maxReadSize;
};

qint64 UploadDataGenerator::writeData(const char* data, qint64 maxSize) {
  logger.debug() << "Write data";
  Q_UNUSED(data);
  Q_UNUSED(maxSize);

  return -1;
};
