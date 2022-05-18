/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "logger.h"
#include "loghandler.h"

#include <QJsonDocument>
#include <QMetaEnum>

Logger::Logger(const QString& module, const QString& className)
    : Logger(QStringList({module}), className) {}

Logger::Logger(const QStringList& modules, const QString& className)
    : m_modules(modules), m_className(className) {}

Logger::Log Logger::error() { return Log(this, LogLevel::Error); }
Logger::Log Logger::warning() { return Log(this, LogLevel::Warning); }
Logger::Log Logger::info() { return Log(this, LogLevel::Info); }
Logger::Log Logger::debug() { return Log(this, LogLevel::Debug); }

Logger::Log::Log(Logger* logger, LogLevel logLevel)
    : m_logger(logger), m_logLevel(logLevel), m_data(new Data()) {}

Logger::Log::~Log() {
  LogHandler::messageHandler(m_logLevel, m_logger->modules(),
                             m_logger->className(), m_data->m_buffer.trimmed());
  delete m_data;
}

#define CREATE_LOG_OP_REF(x)                  \
  Logger::Log& Logger::Log::operator<<(x t) { \
    m_data->m_ts << t << ' ';                 \
    return *this;                             \
  }

CREATE_LOG_OP_REF(uint64_t);
CREATE_LOG_OP_REF(const char*);
CREATE_LOG_OP_REF(const QString&);
CREATE_LOG_OP_REF(const QByteArray&);
CREATE_LOG_OP_REF(void*);

#undef CREATE_LOG_OP_REF

Logger::Log& Logger::Log::operator<<(const QStringList& t) {
  m_data->m_ts << '[' << t.join(",") << ']' << ' ';
  return *this;
}

Logger::Log& Logger::Log::operator<<(const QJsonObject& t) {
  m_data->m_ts << QJsonDocument(t).toJson(QJsonDocument::Indented) << ' ';
  return *this;
}

Logger::Log& Logger::Log::operator<<(QTextStreamFunction t) {
  m_data->m_ts << t;
  return *this;
}

QString Logger::sensitive(const QString& input) {
#ifdef MVPN_DEBUG
  return input;
#else
  Q_UNUSED(input);
  return QString(8, 'X');
#endif
}

QString Logger::keys(const QString& input) {
#ifdef MVPN_DEBUG
  if (input.length() < 12) {
    return input;
  } else {
    return input.left(5) + "..." + input.right(5);
  }
#else
  Q_UNUSED(input);
  return QString(8, 'X');
#endif
}

void Logger::Log::addMetaEnum(quint64 value, const QMetaObject* meta,
                              const char* name) {
  QMetaEnum me = meta->enumerator(meta->indexOfEnumerator(name));

  QString out;
  QTextStream ts(&out);

  if (const char* scope = me.scope()) {
    ts << scope << "::";
  }

  const char* key = me.valueToKey(value);
  const bool scoped = me.isScoped();
  if (scoped || !key) {
    ts << me.enumName() << (!key ? "(" : "::");
  }

  if (key) {
    ts << key;
  } else {
    ts << value << ")";
  }

  m_data->m_ts << out;
}
