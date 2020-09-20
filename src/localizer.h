#ifndef LOCALIZER_H
#define LOCALIZER_H

#include <QTranslator>

class Localizer
{
public:
    void initialize();

private:
    QTranslator m_translator;

    QStringList m_languages;
};

#endif // LOCALIZER_H
