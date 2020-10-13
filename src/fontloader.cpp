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

    QFontDatabase a;

    qDebug() << a.families();
    qDebug() << a.bold("MetropolisSemiBold", "Regular");
}
