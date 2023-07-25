// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Script that validates the generate addon RCC files
// can all be loaded by a Qt application.

#include <QDebug>
#include <QDir>
#include <QResource>

int main(int argc, char* argv[]) {
  if (argc != 2) {
    qDebug() << "Usage: " << argv[0] << " <directory_path>";
    return 1;
  }

  QString directoryPath = argv[1];
  QDir directory(directoryPath);

  if (!directory.exists()) {
    qDebug() << "\033[1;31mDirectory does not exist: " << directoryPath
             << "\033[0m\n";
    return 1;
  }

  directory.setFilter(QDir::Files | QDir::NoDotAndDotDot);
  QFileInfoList fileInfoList = directory.entryInfoList();

  bool foundBrokenAddons = false;

  foreach (const QFileInfo& fileInfo, fileInfoList) {
    QString filePath = fileInfo.absoluteFilePath();
    QString fileExtension = fileInfo.suffix();
    if (fileExtension == "rcc") {
      if (!QResource::registerResource(filePath)) {
        qDebug() << "\033[1;31mGenerated resource file" << filePath
                 << "cannot be registered by Qt.\033[0m";
        foundBrokenAddons = true;
        continue;
      }

      qDebug() << "Generated resource file" << filePath << "is valid.";
    }
  }

  if (foundBrokenAddons) {
    return 1;
  } else {
    return 0;
  }
}
