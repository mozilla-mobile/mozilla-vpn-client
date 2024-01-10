/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addondirectory.h"

#include <QByteArray>
#include <QDir>
#include <QSaveFile>
#include <QStandardPaths>
#include <QString>

#include "leakdetector.h"
#include "logging/logger.h"

namespace {
Logger logger("AddonDirectory");

QString rootAppFolder() {
#ifdef MZ_WASM
  // https://wiki.qt.io/Qt_for_WebAssembly#Files_and_local_file_system_access
  return "/";
#elif defined(UNIT_TEST)
  return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
#else
  return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#endif
}
}  // namespace

AddonDirectory::AddonDirectory() {
  MZ_COUNT_CTOR(AddonDirectory);

  QDir addonDirectory(rootAppFolder());
  if (!addonDirectory.exists(ADDON_FOLDER) &&
      !addonDirectory.mkpath(ADDON_FOLDER)) {
    logger.error() << "Unable to create the addon folder" << rootAppFolder();
    return;
  }
}

AddonDirectory::~AddonDirectory() { MZ_COUNT_DTOR(AddonDirectory); }

// static
bool AddonDirectory::getDirectory(QDir* dir) {
  QDir addonDirectory(rootAppFolder());
  if (!addonDirectory.exists(ADDON_FOLDER)) {
    return false;
  }

  if (!addonDirectory.cd(ADDON_FOLDER)) {
    logger.warning() << "Unable to open the addons folder";
    return false;
  }

  *dir = addonDirectory;
  return true;
}

// static
bool AddonDirectory::readFile(const QString& fileName, QByteArray* contents) {
  QDir dir;
  if (!getDirectory(&dir)) {
    return false;
  }

  QFile file(dir.filePath(fileName));

  if (!file.exists()) {
    logger.info() << "File" << file.fileName() << "does not exist yet";
    return false;
  }

  if (!file.open(QIODevice::ReadOnly)) {
    logger.warning() << "Unable to open file:" << file.fileName() << "\n"
                     << file.errorString();
    return false;
  }

  *contents = file.readAll();
  return true;
}

// static
bool AddonDirectory::writeToFile(const QString& fileName,
                                 const QByteArray& contents) {
  QDir dir;
  if (!getDirectory(&dir)) {
    return false;
  }

  QSaveFile file(dir.filePath(fileName));

  if (!file.open(QIODevice::WriteOnly)) {
    logger.warning() << "Unable to open file:" << file.fileName() << "\n"
                     << file.errorString();
    return false;
  }

  if (!file.write(contents)) {
    logger.warning() << "Unable to write to file:" << file.fileName();
    return false;
  }

  if (!file.commit()) {
    logger.warning() << "Unable to commit to file:" << file.fileName();
    return false;
  }

  return true;
}

// static
bool AddonDirectory::deleteFile(const QString& fileName) {
  QDir dir;
  if (!getDirectory(&dir)) {
    return false;
  }

  if (!dir.exists(fileName)) {
    return false;
  }

  if (!dir.remove(fileName)) {
    logger.warning() << "Unable to delete file:" << fileName;
    return false;
  }

  return true;
}

// static
void AddonDirectory::reset() {
  QDir dir;
  if (!getDirectory(&dir)) {
    return;
  }

  dir.setNameFilters(QStringList() << "*");
  dir.setFilter(QDir::Files);
  foreach (QString dirFile, dir.entryList()) {
    // This comment is here to make the linter happy.
    dir.remove(dirFile);
  }
}
