/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "resourceloaderutils.h"

constexpr int32_t FILENAME_LENGTH_MAX = 30;

// static
bool ResourceLoaderUtils::isFileNameLengthValid(int32_t fileNameLength) {
  return fileNameLength > 0 && fileNameLength <= FILENAME_LENGTH_MAX;
}

// static
bool ResourceLoaderUtils::isFileNameValid(const QString& fileName) {
  if (!isFileNameLengthValid(fileName.length())) {
    return false;
  }

  if (fileName[0] == '.') {
    return false;
  }

  for (QChar ch : fileName) {
    if ((ch < 'a' || ch > 'z') && (ch < 'A' || ch > 'Z') &&
        (ch < '0' || ch > '9') && ch != '.' && ch != '_' && ch != '-') {
      return false;
    }
  }

  return true;
}
