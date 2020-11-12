/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QTextStream>

constexpr const char *LOG_CAPTIVEPORTAL = "captiveportal";
constexpr const char *LOG_CONTROLLER = "controller";
constexpr const char *LOG_MAIN = "main";
constexpr const char *LOG_MODEL = "model";
constexpr const char *LOG_NETWORKING = "networking";

#ifdef QT_DEBUG
constexpr const char *LOG_INSPECTOR = "inspector";
#endif

#ifdef MVPN_IOS
constexpr const char *LOG_IAP = "iap";
constexpr const char *LOG_IOS = "ios";
#endif

#if defined(MVPN_LINUX) || defined(MVPN_ANDROID)
constexpr const char *LOG_LINUX = "linux";
#endif

#if __APPLE__
constexpr const char *LOG_MACOS = "macos";
#endif

class QNetworkReply;

class Logger
{
public:
    Logger(const QString &module, const QString &className);
    Logger(const QStringList &modules, const QString &className);

    const QStringList &modules() const { return m_modules; }
    const QString &className() const { return m_className; }

    class Log
    {
    public:
        Log(Logger *logger);
        ~Log();

        Log &operator<<(uint64_t t);
        Log &operator<<(const char *t);
        Log &operator<<(const QString &t);
        Log &operator<<(const QStringList &t);
        Log &operator<<(const QByteArray &t);
        Log &operator<<(QTextStreamFunction t);

    private:
        Logger *m_logger;

        struct Data
        {
            Data() : m_ts(&m_buffer, QIODevice::WriteOnly) {}

            QString m_buffer;
            QTextStream m_ts;
        };

        Data *m_data;
    };

    Log log();

private:
    QStringList m_modules;
    QString m_className;
};

#endif // LOGGER_H
