#ifndef PINGSENDER_H
#define PINGSENDER_H

#include <QObject>
#include <QThread>

class PingSender final : public QObject
{
    Q_OBJECT

public:
    explicit PingSender(QObject *parent);

    ~PingSender();

    void send(const QString &destination);

    void stop();

signals:
    void completed();

    // internal only
    void sendPing(const QString &destination);
    void stopPing();

private Q_SLOTS:
    void pingFailed();
    void pingSucceeded();

private:
    QThread m_workerThread;

    bool m_active = false;
};

#endif // PINGSENDER_H
