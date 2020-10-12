/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "localizer.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLocale>

void Localizer::initialize(const QString& code)
{
    m_code = code;
    if(code.isEmpty()){
        QLocale locale = QLocale::system();
        m_code = locale.bcp47Name();
    }
    qDebug() << "Localizer initializing:" << m_code;


    loadLanguage(m_code);

    QCoreApplication::installTranslator(&m_translator);
    QDir dir(":/i18n");
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

void Localizer::loadLanguage(const QString& code)
{
    QLocale locale = QLocale(code);
    if (code.isEmpty()) {
        locale = QLocale::system();
    }
    QLocale::setDefault(locale);

    if (!m_translator.load(locale, "mozillavpn", "_", ":/i18n")) {
        qDebug() << "Loading the locale failed." << "code";
    }
}

QString Localizer::languageName(const QString &code) const
{
    QLocale locale(code);
    if (code.isEmpty()) {
        locale = QLocale::system();
    }
    if (locale.language() == QLocale::C) {
        return "English (US)";
    }

    return QLocale::languageToString(locale.language());
}

QString Localizer::localizedLanguageName(const QString &code) const
{
    QLocale locale(code);
    if (code.isEmpty()) {
        locale = QLocale::system();
    }
    if (locale.language() == QLocale::C) {
        return "English (US)";
    }

    return locale.nativeLanguageName();
}

QHash<int, QByteArray> Localizer::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[LanguageRole] = "language";
    roles[LocalizedLanguageRole] = "localizedLanguage";
    roles[CodeRole] = "code";
    return roles;
}

int Localizer::rowCount(const QModelIndex &) const
{
    return m_languages.count();
}

QVariant Localizer::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case LanguageRole:
        return QVariant(localizedLanguageName(m_languages.at(index.row())));

    case LocalizedLanguageRole:
        return QVariant(languageName(m_languages.at(index.row())));

    case CodeRole:
        return QVariant(m_languages.at(index.row()));

    default:
        return QVariant();
    }
}
