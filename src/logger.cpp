/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "logger.h"
#include "loghandler.h"

Logger::Logger(const QString &module, const QString &className)
    : Logger(QStringList({module}), className)
{}

Logger::Logger(const QStringList &modules, const QString &className)
    : m_modules(modules), m_className(className)
{}

Logger::Log Logger::log()
{
    return Log(this);
}

Logger::Log::Log(Logger *logger) : m_logger(logger), m_ts(&m_buffer, QIODevice::WriteOnly) {}

Logger::Log::Log(const Log &o)
{
    Q_UNUSED(o);
}

Logger::Log::~Log()
{
    LogHandler::messageHandler(m_logger->modules(), m_logger->className(), m_buffer.trimmed());
}

#define CREATE_LOG_OP_REF(x) \
    Logger::Log &Logger::Log::operator<<(x t) \
    { \
        m_ts << t << ' '; \
        return *this; \
    }

CREATE_LOG_OP_REF(uint64_t);
CREATE_LOG_OP_REF(const char *);
CREATE_LOG_OP_REF(const QString &);
CREATE_LOG_OP_REF(const QByteArray &);

#undef CREATE_LOG_OP_REF

Logger::Log &Logger::Log::operator<<(const QStringList &t)
{
    m_ts << '[' << t.join(",") << ']' << ' ';
    return *this;
}

Logger::Log &Logger::Log::operator<<(QTextStreamFunction t)
{
    m_ts << t;
    return *this;
}
