/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONDIRECTORY_H
#define ADDONDIRECTORY_H

#include <QObject>

class QString;
class QByteArray;
class QDir;

constexpr const char* ADDON_FOLDER = "addons";

class AddonDirectory final {
 public:
  AddonDirectory();
  ~AddonDirectory();

  static bool getDirectory(QDir* dir);

  static bool readFile(const QString& fileName, QByteArray* contents);
  static bool writeToFile(const QString& fileName, const QByteArray& contents);
  static bool deleteFile(const QString& fileName);

  static void reset();
};

#endif  // ADDONDIRECTORY_H
