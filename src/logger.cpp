/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "logger.h"

#include <QDate>
#include <QDebug>
#include <QMessageLogContext>
#include <QString>

constexpr int LOG_MAX = 10000;

static Logger logger;

// static
Logger *Logger::instance()
{
    return &logger;
}

// static
void Logger::messageHandler(QtMsgType type,
                            const QMessageLogContext &context,
                            const QString &message)
{
    logger.m_logs.append(Log(type, context.file, context.function, context.line, message));

    while (logger.m_logs.count() >= LOG_MAX) {
        logger.m_logs.removeAt(0);
    }

    QTextStream out(stderr);
    prettyOutput(out, logger.m_logs.last());
}

// static
void Logger::prettyOutput(QTextStream &out, const Logger::Log &log)
{
    out << "[" << log.m_dateTime.toString("dd.MM.yyyy hh:mm:ss.zzz") << "] ";

    switch (log.m_type) {
    case QtDebugMsg:
        out << "Debug: ";
        break;
    case QtInfoMsg:
        out << "Info: ";
        break;
    case QtWarningMsg:
        out << "Warning: ";
        break;
    case QtCriticalMsg:
        out << "Critical: ";
        break;
    case QtFatalMsg:
        out << "Fatal: ";
        break;
    default:
        out << "?!?: ";
        break;
    }

    out << log.m_message;

    if (!log.m_file.isEmpty() || !log.m_function.isEmpty()) {
        out << " (";

        if (!log.m_file.isEmpty()) {
            int pos = log.m_file.lastIndexOf("/");
            out << log.m_file.right(log.m_file.length() - pos - 1) << ":" << log.m_line;
            if (!log.m_function.isEmpty()) {
                out << ", ";
            }
        }

        if (!log.m_function.isEmpty()) {
            out << log.m_function;
        }

        out << ")";
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    out << Qt::endl;
#else
    out << endl;
#endif
}
