/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "fontloader.h"
#include "logger.h"

#include <QDir>
#include <QFontDatabase>

namespace {
Logger logger(LOG_MAIN, "FontLoader");
}

// static
void FontLoader::loadFonts() {
  QDir dir(":/nebula/resources/fonts");
  QStringList files = dir.entryList();
  for (const QString& file : files) {
    logger.debug() << "Loading font:" << file;
    int id =
        QFontDatabase::addApplicationFont(":/nebula/resources/fonts/" + file);
    logger.debug() << "Result:" << id;
  }
}
