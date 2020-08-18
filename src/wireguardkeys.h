#ifndef WireguardKeys_H
#define WireguardKeys_H

#include <QObject>
#include <QPointer>
#include <QProcess>

class QFile;

class WireguardKeys : public QObject
{
    Q_OBJECT

public:
    static WireguardKeys *generateKeys(QObject *parent);

private:
    static QString binary();

    explicit WireguardKeys(QObject *parent) : QObject(parent) {}

    void generatePrivateKey();
    void generatePublicKey();

signals:
    void keysGenerated(const QString &privateKey, const QString &publicKey);

private Q_SLOTS:
    void privateKeyGenerated(int exitCode, QProcess::ExitStatus exitStatus);
    void publicKeyGenerated(int exitCode, QProcess::ExitStatus exitStatus);
    void publicKeyGenStarted();

    void errorOccurred(QProcess::ProcessError);

private:
    QPointer<QProcess> m_process;

    QByteArray m_privateKey;
    QByteArray m_publicKey;
};

#endif // WireguardKeys_H
