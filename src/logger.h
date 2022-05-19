/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOGGER_H
#define LOGGER_H

#include "loglevel.h"

#include <QIODevice>
#include <QObject>
#include <QString>
#include <QTextStream>

constexpr const char* LOG_ADJUST = "adjust";
constexpr const char* LOG_CAPTIVEPORTAL = "captiveportal";
constexpr const char* LOG_CONTROLLER = "controller";
constexpr const char* LOG_IAP = "iap";
constexpr const char* LOG_INSPECTOR = "inspector";
constexpr const char* LOG_MAIN = "main";
constexpr const char* LOG_MODEL = "model";
constexpr const char* LOG_NETWORKING = "networking";
constexpr const char* LOG_SERVER = "server";
constexpr const char* LOG_CRASHREPORTER = "crashreporter";

#if defined(MVPN_LINUX) || defined(MVPN_ANDROID)
constexpr const char* LOG_LINUX = "linux";
#endif

#ifdef MVPN_WINDOWS
constexpr const char* LOG_WINDOWS = "windows";
#endif

#if __APPLE__ || defined(MVPN_WASM)
constexpr const char* LOG_MACOS = "macos";
constexpr const char* LOG_IOS = "ios";
#endif

#if defined(MVPN_ANDROID) || defined(UNIT_TEST)
constexpr const char* LOG_ANDROID = "android";
#endif

class QJsonObject;

class Logger {
 public:
  Logger(const QString& module, const QString& className);
  Logger(const QStringList& modules, const QString& className);

  const QStringList& modules() const { return m_modules; }
  const QString& className() const { return m_className; }

  class Log {
   public:
    Log(Logger* logger, LogLevel level);
    ~Log();

    Log& operator<<(uint64_t t);
    Log& operator<<(const char* t);
    Log& operator<<(const QString& t);
    Log& operator<<(const QStringList& t);
    Log& operator<<(const QByteArray& t);
    Log& operator<<(const QJsonObject& t);
    Log& operator<<(QTextStreamFunction t);
    Log& operator<<(void* t);

    // Q_ENUM
    template <typename T>
    typename std::enable_if<QtPrivate::IsQEnumHelper<T>::Value, Log&>::type
    operator<<(T t) {
      const QMetaObject* meta = qt_getEnumMetaObject(t);
      const char* name = qt_getEnumName(t);
      addMetaEnum(typename QFlags<T>::Int(t), meta, name);
      return *this;
    }

   private:
    void addMetaEnum(quint64 value, const QMetaObject* meta, const char* name);

    Logger* m_logger;
    LogLevel m_logLevel;

    struct Data {
      Data() : m_ts(&m_buffer, QIODevice::WriteOnly) {}

      QString m_buffer;
      QTextStream m_ts;
    };

    Data* m_data;
  };

  Log error();
  Log warning();
  Log info();
  Log debug();

  // Use this to log sensitive data such as IP address, session tokens, and etc.
  // When compiled with debug, this allows the sensitive data to be logged.
  QString sensitive(const QString& input);

  // Use this to log keys, which should always be obscured.
  // When compiled with debug, this truncates the keys instead for readability.
  QString keys(const QString& input);

 private:
  QStringList m_modules;
  QString m_className;
};

#endif  // LOGGER_H
