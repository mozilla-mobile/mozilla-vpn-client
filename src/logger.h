#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QTextStream>

class QNetworkReply;

class Logger
{
public:
    Logger(const QString &module);

    const QString &module() const { return m_module; }

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
};

#endif // LOGGER_H
