#include "localizer.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLocale>

namespace {

QString findPath()
{
    {
        QFileInfo fi(TRANSLATIONS_PATH);
        if (fi.exists()) {
            return QString(TRANSLATIONS_PATH);
        }
    }

    qDebug() << QDir::currentPath();
#ifdef QT_DEBUG
    {
        QFileInfo fi("../translations");
        if (fi.exists()) {
            return QString("../translations");
        }
    }

    {
        QFileInfo fi("./translations");
        if (fi.exists()) {
            return QString("./translations");
        }
    }
#endif

    return QString();
}

} // anonymous namespace

void Localizer::initialize()
{
    qDebug() << "Localizer initializing";

    QString translationPath = findPath();
    if (translationPath.isEmpty()) {
        qDebug() << "Unable to localize the translation files.";
        return;
    }

    qDebug() << "Looking for translation files in path: " << translationPath;

    if (!m_translator.load(QLocale(), "mozillavpn", "_", translationPath)) {
        qDebug() << "Loading the locale failed.";
        return;
    }

    QCoreApplication::installTranslator(&m_translator);

    QDir dir(translationPath);
    QStringList files = dir.entryList();
    for (QStringList::ConstIterator i = files.begin(); i != files.end(); ++i) {
        if (!i->endsWith(".qm")) {
            continue;
        }

        QStringList parts = i->split(".");
        Q_ASSERT(parts.length() == 2);

        parts = parts[0].split("_");
        Q_ASSERT(parts.length() == 2);

        m_languages.append(parts.at(1));
    }
}
