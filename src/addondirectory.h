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

class AddonDirectory final : public QObject {
  Q_OBJECT

 public:
  AddonDirectory();

  bool getDirectory(QDir* dir);

  bool readFile(const QString& fileName, QByteArray* contents);
  bool writeToFile(const QString& fileName, const QByteArray& contents);
  bool deleteFile(const QString& fileName);

#ifdef UNIT_TEST
  void testReset();
#endif
};

#endif  // ADDONDIRECTORY_H
