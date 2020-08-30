#ifndef LINUXPINGSENDWORKER_H
#define LINUXPINGSENDWORKER_H

#include <QObject>

class QSocketNotifier;

class LinuxPingSendWorker : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void sendPing(const QString &destination);
    void stopPing();

signals:
    void pingSucceeded();
    void pingFailed();

private:
    QSocketNotifier *m_socketNotifier;
};

#endif // LINUXPINGSENDWORKER_H
