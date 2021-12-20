/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RESOURCELOADERUTILS_H
#define RESOURCELOADERUTILS_H

#include <QObject>

class ResourceLoaderUtils final {
 public:
  static bool isFileNameLengthValid(int32_t fileNameLength);
  static bool isFileNameValid(const QString& fileName);
};

#endif  // RESOURCELOADERUTILS_H
