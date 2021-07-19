/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QTextStream>

constexpr const char* LOG_CAPTIVEPORTAL = "captiveportal";
constexpr const char* LOG_CONTROLLER = "controller";
constexpr const char* LOG_MAIN = "main";
constexpr const char* LOG_MODEL = "model";
constexpr const char* LOG_NETWORKING = "networking";
constexpr const char* LOG_INSPECTOR = "inspector";
constexpr const char* LOG_SERVER = "server";

#ifdef MVPN_IOS
constexpr const char* LOG_IAP = "iap";
#endif

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

class Logger {
 public:
  Logger(const QString& module, const QString& className);
  Logger(const QStringList& modules, const QString& className);

  const QStringList& modules() const { return m_modules; }
  const QString& className() const { return m_className; }

  class Log {
   public:
    Log(Logger* logger, QtMsgType type = QtMsgType::QtDebugMsg);
    ~Log();

    Log& operator<<(uint64_t t);
    Log& operator<<(const char* t);
    Log& operator<<(const QString& t);
    Log& operator<<(const QStringList& t);
    Log& operator<<(const QByteArray& t);
    Log& operator<<(QTextStreamFunction t);
    Log& operator<<(void* t);

   private:
    Logger* m_logger;
    QtMsgType m_type;

    struct Data {
      Data() : m_ts(&m_buffer, QIODevice::WriteOnly) {}

      QString m_buffer;
      QTextStream m_ts;
    };

    Data* m_data;
  };

  Log log();
  Log info();
  Log debug();
  Log warning();

 private:
  QStringList m_modules;
  QString m_className;
};

#endif  // LOGGER_H
