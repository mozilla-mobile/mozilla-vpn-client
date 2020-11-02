/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "loghandler.h"

#include <QDate>
#include <QMessageLogContext>
#include <QMutexLocker>
#include <QProcessEnvironment>
#include <QString>
#include <QTextStream>

constexpr int LOG_MAX = 10000;

namespace {
LogHandler *sLogHandler;
}

// static
LogHandler *LogHandler::instance()
{
    return maybeCreate();
}

// static
void LogHandler::messageQTHandler(QtMsgType type,
                                  const QMessageLogContext &context,
                                  const QString &message)
{
    maybeCreate()->addLog(Log(type, context.file, context.function, context.line, message));
}

// static
void LogHandler::messageHandler(const QStringList &modules,
                                const QString &className,
                                const QString &message)
{
    maybeCreate()->addLog(Log(modules, className, message));
}

// static
LogHandler *LogHandler::maybeCreate()
{
    if (!sLogHandler) {
        sLogHandler = new LogHandler();
    }
    return sLogHandler;
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
        out << "(" << log.m_modules.join("|") << " - " << log.m_className << ") " << log.m_message;
    }

    out << Qt::endl;
}

LogHandler::LogHandler()
{
    QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
    if (pe.contains("MOZVPN_LOG")) {
        QStringList parts = pe.value("MOZVPN_LOG").split(",");
        for (const QString &part : parts) {
            m_modules.append(part.trimmed());
        }
    }
}

void LogHandler::addLog(const Log &log)
{
    QMutexLocker lock(&m_mutex);

    if (!matchModule(log, lock)) {
        return;
    }

    m_logs.append(log);

    while (m_logs.count() >= LOG_MAX) {
        m_logs.removeAt(0);
    }

    QTextStream out(stderr);
    prettyOutput(out, m_logs.last());
}

bool LogHandler::matchModule(const Log &log, const QMutexLocker &proofOfLock) const
{
    Q_UNUSED(proofOfLock);

    // Let's include QT logs always.
    if (log.m_fromQT) {
        return true;
    }

    // If no modules has been specified, let's include all.
    if (m_modules.isEmpty()) {
        return true;
    }

    for (const QString &module : log.m_modules) {
        if (m_modules.contains(module)) {
            return true;
        }
    }

    return false;
}

const QVector<LogHandler::Log> &LogHandler::logs()
{
    QMutexLocker lock(&m_mutex);
    return m_logs;
}
