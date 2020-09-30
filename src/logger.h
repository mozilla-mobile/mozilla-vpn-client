/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>

class QTextStream;

class Logger : public QObject
{
    Q_OBJECT

public:
    static Logger *instance();

    static void messageHandler(QtMsgType type,
                               const QMessageLogContext &context,
                               const QString &message);

    Q_INVOKABLE void viewLogs();

private:
    struct Log
    {
        Log() = default;

        Log(QtMsgType type,
            const QString &file,
            const QString &function,
            uint32_t line,
            const QString &message)
            : m_file(file), m_function(function), m_message(message), m_type(type), m_line(line)
        {}

        QString m_file;
        QString m_function;
        QString m_message;
        QtMsgType m_type;
        uint32_t m_line;
    };

    static void prettyOutput(QTextStream &out, const Logger::Log &log);

    QVector<Log> m_logs;
};

#endif // LOGGER_H
