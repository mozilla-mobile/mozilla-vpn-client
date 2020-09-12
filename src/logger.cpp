#include "logger.h"

#include <QDate>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QMessageLogContext>
#include <QStandardPaths>
#include <QString>
#include <QUrl>
#include <QVector>

constexpr int LOG_MAX = 10000;

static Logger logger;

// static
Logger *Logger::instance()
{
    return &logger;
}

// static
void Logger::messageHandler(QtMsgType type,
                            const QMessageLogContext &context,
                            const QString &message)
{
    logger.m_logs.append(Log(type, context.file, context.function, context.line, message));

    while (logger.m_logs.count() >= LOG_MAX) {
        logger.m_logs.removeAt(0);
    }

#ifdef QT_DEBUG
    QTextStream out(stderr);
    prettyOutput(out, logger.m_logs.last());
#elif MACOS_INTEGRATION
    // TODO: we should not print on stderr in release.
    QTextStream out(stderr);
    prettyOutput(out, logger.m_logs.last());
#endif
}

void Logger::viewLogs()
{
    qDebug() << "View logs";

    QString filename;
    QDate now = QDate::currentDate();

    QTextStream(&filename) << "mozillavpn-" << now.year() << "-" << now.month() << "-" << now.day()
                           << ".txt";

    QStandardPaths::StandardLocation location;

    if (QFileInfo::exists(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation))) {
        location = QStandardPaths::DesktopLocation;
    } else if (QFileInfo::exists(QStandardPaths::writableLocation(QStandardPaths::HomeLocation))) {
        location = QStandardPaths::HomeLocation;
    } else if (QFileInfo::exists(QStandardPaths::writableLocation(QStandardPaths::TempLocation))) {
        location = QStandardPaths::TempLocation;
    } else {
        qWarning() << "No Desktop, no Home, no Temp folder. Unable to store the log files.";
        return;
    }

    QDir logDir(QStandardPaths::writableLocation(location));
    QString logFile = logDir.filePath(filename);

    if (QFileInfo::exists(logFile)) {
        qDebug() << logFile << "exists. Let's try a new filename";

        for (uint32_t i = 1;; ++i) {
            QString filename;
            QTextStream(&filename) << "mozillavpn-" << now.year() << "-" << now.month() << "-"
                                   << now.day() << "_" << i << ".txt";
            logFile = logDir.filePath(filename);
            if (!QFileInfo::exists(logFile)) {
                qDebug() << "Filename found!" << i;
                break;
            }
        }
    }

    qDebug() << "Writing logs into: " << logFile;

    {
        QFile file(logFile);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Failed to open the logfile";
            return;
        }

        QTextStream out(&file);
        for (QVector<Log>::ConstIterator i = m_logs.begin(); i != m_logs.end(); ++i) {
            prettyOutput(out, *i);
        }

        file.close();
    }

    qDebug() << "Opening the logFile somehow";
    QUrl logFileUrl = QUrl::fromLocalFile(logFile);
    QDesktopServices::openUrl(logFileUrl);
}

// static
void Logger::prettyOutput(QTextStream &out, const Logger::Log &log)
{
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
            out << log.m_file.right(log.m_file.length() - pos - 1) << ":" << log.m_line;
            if (!log.m_function.isEmpty()) {
                out << ", ";
            }
        }

        if (!log.m_function.isEmpty()) {
            out << log.m_function;
        }

        out << ")";
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    out << Qt::endl;
#else
    out << endl;
#endif
}
