/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "loghandler.h"

#include <QDate>
#include <QMessageLogContext>
#include <QString>
#include <QTextStream>

constexpr int LOG_MAX = 10000;

static LogHandler logHandler;

// static
LogHandler *LogHandler::instance()
{
    return &logHandler;
}

// static
void LogHandler::messageQTHandler(QtMsgType type,
                                  const QMessageLogContext &context,
                                  const QString &message)
{
    logHandler.m_logs.append(Log(type, context.file, context.function, context.line, message));

    while (logHandler.m_logs.count() >= LOG_MAX) {
        logHandler.m_logs.removeAt(0);
    }

    QTextStream out(stderr);
    prettyOutput(out, logHandler.m_logs.last());
}

// static
void LogHandler::messageHandler(const QString &module, const QString &message)
{
    logHandler.m_logs.append(Log(module, message));

    while (logHandler.m_logs.count() >= LOG_MAX) {
        logHandler.m_logs.removeAt(0);
    }

    QTextStream out(stderr);
    prettyOutput(out, logHandler.m_logs.last());
}

// static
void LogHandler::prettyOutput(QTextStream &out, const LogHandler::Log &log)
{
    out << "[" << log.m_dateTime.toString("dd.MM.yyyy hh:mm:ss.zzz") << "] ";

    if (log.m_fromQT) {
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
                out << log.m_file.right(log.m_file.length() - pos - 1);

                if (log.m_line >= 0) {
                    out << ":" << log.m_line;
                }

                if (!log.m_function.isEmpty()) {
                    out << ", ";
                }
            }

            if (!log.m_function.isEmpty()) {
                out << log.m_function;
            }

            out << ")";
        }
    } else {
        out << "(" << log.m_module << ") " << log.m_message;
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    out << Qt::endl;
#else
    out << endl;
#endif
}
