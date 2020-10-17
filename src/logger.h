#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QTextStream>

constexpr const char *LOG_CAPTIVEPORTAL = "captiveportal";
constexpr const char *LOG_CONTROLLER = "controller";
constexpr const char *LOG_MAIN = "main";
constexpr const char *LOG_MODEL = "model";
constexpr const char *LOG_NETWORKING = "networking";

#ifdef IOS_INTEGRATION
constexpr const char *LOG_IAP = "iap";
#endif

#ifdef __linux__
constexpr const char *LOG_LINUX = "linux";
#endif

#ifdef __APPLE__
constexpr const char *LOG_MACOS = "macos";
#endif

class QNetworkReply;

class Logger
{
public:
    Logger(const QString &module, const QString& className);

    const QString &module() const { return m_module; }
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

    private:
        Logger *m_logger;
        QString m_buffer;
        QTextStream m_ts;
    };

    Log log();

private:
    QString m_module;
    QString m_className;
};

#endif // LOGGER_H
