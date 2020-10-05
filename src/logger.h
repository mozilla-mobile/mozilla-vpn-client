/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOGGER_H
#define LOGGER_H

#include <QDateTime>
#include <QObject>
#include <QVector>

class QTextStream;

class Logger final : public QObject
{
    Q_OBJECT

public:
    struct Log
    {
        Log() = default;

        Log(QtMsgType type,
            const QString &file,
            const QString &function,
            uint32_t line,
            const QString &message)
            : m_dateTime(QDateTime::currentDateTime()), m_file(file), m_function(function),
              m_message(message), m_type(type), m_line(line)
        {}

        QDateTime m_dateTime;
        QString m_file;
        QString m_function;
        QString m_message;
        QtMsgType m_type;
        uint32_t m_line;
    };

    static Logger *instance();

    static void messageHandler(QtMsgType type,
                               const QMessageLogContext &context,
                               const QString &message);

    static void prettyOutput(QTextStream &out, const Logger::Log &log);

    const QVector<Log> &logs() const { return m_logs; }

private:
    QVector<Log> m_logs;
};

#endif // LOGGER_H
