/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QString>
#include <QStringList>

class CommandLineParser final
{
public:
    struct Option
    {
        Option(const char *a_short, const char *a_long, const char *description)
            : m_short(a_short), m_long(a_long), m_description(description)
        {}

        const char *m_short;
        const char *m_long;
        const char *m_description;

        bool m_set = false;
    };

    void parse(int argc, char *argv[]);

    void parse(QStringList &tokens, QList<Option *> &options, bool hasCommands);

    static void unknownOption(const QString &option,
                              const QString &app,
                              QList<Option *> &options,
                              bool hasCommands);

    static Option helpOption();

    static void showHelp(const QString &app,
                         const QList<Option *> &options,
                         bool hasCommands,
                         bool compact);

private:
    static bool parseOptions(QStringList &tokens, QList<Option *> &options);
    static bool parseOption(const QString &option, bool shortOption, QList<Option *> &options);
};

#endif // COMMANDLINEPARSER_H
