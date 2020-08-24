#ifndef TASKAUTHENTICATIONVERIFIER_H
#define TASKAUTHENTICATIONVERIFIER_H

#include <QObject>
#include <QString>

class TaskAuthenticationVerifier final : public QObject
{
    Q_OBJECT

public:
    TaskAuthenticationVerifier(QObject *parent,
                               const QString &verificationUrl,
                               uint32_t pollInterval);

private:
    void run();

private Q_SLOTS:
    void timerExpired();

signals:
    void completed(const QByteArray &);

private:
    QString m_verificationUrl;
    uint32_t m_pollInterval;
};

#endif // TASKAUTHENTICATIONVERIFIER_H
