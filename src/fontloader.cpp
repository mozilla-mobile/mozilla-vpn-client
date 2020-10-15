/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "fontloader.h"

#include <QDebug>
#include <QDir>
#include <QFontDatabase>

// static
void FontLoader::loadFonts()
{
    QDir dir(":/resources/fonts");
    QStringList files = dir.entryList();
    for (QStringList::ConstIterator i = files.begin(); i != files.end(); ++i) {
        qDebug() << "Loading font:" << *i;
        int id = QFontDatabase::addApplicationFont(":/resources/fonts/" + *i);
        qDebug() << "Result:" << id;
    }
}
