/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nulldevice.h"

#include "leakdetector.h"

bool NullDevice::isSequential() const {
  return true;
}

qint64 NullDevice::readData(char *data, qint64 maxSize) {
  Q_UNUSED(data);
  Q_UNUSED(maxSize);
  return -1;
}

qint64 NullDevice::writeData(const char *data, qint64 maxSize) {
  Q_UNUSED(data);
  return maxSize;
}
