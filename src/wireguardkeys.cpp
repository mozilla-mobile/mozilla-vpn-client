#include "wireguardkeys.h"

#include <QDebug>
#include <QStringList>

// TODO:
constexpr const char *WG_BIN = "/Users/baku/Sources/m/wireguard-tools/src/wg";

// static
WireguardKeys *WireguardKeys::generateKeys(QObject *parent)
{
    qDebug() << "Wireguard key generation";

    WireguardKeys *wgh = new WireguardKeys(parent);

    wgh->generatePrivateKey();

    return wgh;
}

void WireguardKeys::generatePrivateKey()
{
    qDebug() << "Generation private key";

    Q_ASSERT(!m_process);
    m_process = new QProcess(this);

    QObject::connect(m_process,
                     SIGNAL(finished(int, QProcess::ExitStatus)),
                     this,
                     SLOT(privateKeyGenerated(int, QProcess::ExitStatus)));

    QObject::connect(m_process,
                     SIGNAL(errorOccurred(QProcess::ProcessError)),
                     this,
                     SLOT(errorOccurred(QProcess::ProcessError)));

    QStringList arguments;
    arguments << "genkey";
    m_process->start(WG_BIN, arguments, QIODevice::ReadOnly);
}

void WireguardKeys::privateKeyGenerated(int exitCode, QProcess::ExitStatus)
{
    Q_ASSERT(m_process);

    qDebug() << "Process exited with code: " << exitCode;
    if (exitCode != 0) {
        // TODO
        return;
    }

    m_privateKey = m_process->readAllStandardOutput();
    delete m_process;

    generatePublicKey();
}

void WireguardKeys::generatePublicKey()
{
    Q_ASSERT(!m_process);
    m_process = new QProcess(this);

    QObject::connect(m_process, SIGNAL(started()), this, SLOT(publicKeyGenStarted()));

    QObject::connect(m_process,
                     SIGNAL(finished(int, QProcess::ExitStatus)),
                     this,
                     SLOT(publicKeyGenerated(int, QProcess::ExitStatus)));

    QObject::connect(m_process,
                     SIGNAL(errorOccurred(QProcess::ProcessError)),
                     this,
                     SLOT(errorOccurred(QProcess::ProcessError)));

    QStringList arguments;
    arguments << "genkey";
    m_process->start(WG_BIN, arguments, QIODevice::ReadOnly);
}

void WireguardKeys::publicKeyGenStarted()
{
    qDebug() << "public generation process has started";

    Q_ASSERT(m_process);
    m_process->write(m_privateKey.constData());
    m_process->closeWriteChannel();
}

void WireguardKeys::publicKeyGenerated(int exitCode, QProcess::ExitStatus)
{
    Q_ASSERT(m_process);

    qDebug() << "Process exited with code: " << exitCode;
    if (exitCode != 0) {
        // TODO
        return;
    }

    m_publicKey = m_process->readAllStandardOutput();
    delete m_process;

    emit keysGenerated(m_privateKey.trimmed(), m_publicKey.trimmed());
}

void WireguardKeys::errorOccurred(QProcess::ProcessError)
{
    qDebug() << "Error!";
    // TODO
}
