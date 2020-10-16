#include "logger.h"
#include "loghandler.h"

Logger::Logger(const QString &module) : m_module(module) {}

Logger::Log Logger::log()
{
    return Log(this);
}

Logger::Log::Log(Logger *logger) : m_logger(logger), m_ts(&m_buffer, QIODevice::WriteOnly) {}

Logger::Log::~Log()
{
    LogHandler::messageHandler(m_logger->module(), m_buffer.trimmed());
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
