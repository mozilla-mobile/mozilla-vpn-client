#include "wgquickprocess.h"
#include "device.h"
#include "keys.h"
#include "server.h"

#include <QDebug>
#include <QProcess>

constexpr const char *WG_QUICK = "wg-quick";

WgQuickProcess::WgQuickProcess(WgQuickProcess::Op op) : m_op(op) {}

void WgQuickProcess::Run(const Server &server, const Device *device, const Keys *keys)
{
    Q_ASSERT(device);
    Q_ASSERT(keys);

    QByteArray content;
    content.append("[Interface]\nPrivateKey = ");
    content.append(keys->privateKey());
    content.append("\nAddress = ");
    content.append(device->ipv4Address());
    content.append(", ");
    content.append(device->ipv6Address());
    content.append("\nDNS = ");
    content.append(server.ipv4Gateway());
    content.append("\n\n[Peer]\nPublicKey = ");
    content.append(server.publicKey());
    content.append("\nEndpoint = ");
    content.append(server.ipv4AddrIn());
    content.append(QString(":%1").arg(server.choosePort()));
    content.append("\nAllowedIPs = 0.0.0.0/0,::0/0\n");

    if (!tmpDir.isValid()) {
        qWarning("Cannot create a temporary directory");
        emit failed();
        return;
    }

    QDir dir(tmpDir.path());
    QFile file(dir.filePath("wg0.conf"));
    if (!file.open(QIODevice::ReadWrite)) {
        qWarning("Unable to create a file in the temporary folder");
        emit failed();
        return;
    }

    qint64 written = file.write(content);
    if (written != content.length()) {
        qWarning("Unable to write the whole configuration file");
        emit failed();
        return;
    }

    file.close();

    QStringList arguments;
    arguments.append(m_op == Up ? "up" : "down");
    arguments.append(file.fileName());

    QProcess *wgQuickProcess = new QProcess(this);

    connect(wgQuickProcess, &QProcess::errorOccurred, [this](QProcess::ProcessError error) {
        qDebug() << "Error occurred" << error;
        deleteLater();
        emit failed();
    });

    connect(wgQuickProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, wgQuickProcess](int exitCode, QProcess::ExitStatus exitStatus) {
                qDebug() << "Execution finished" << exitCode;

                qDebug() << "Output:" << wgQuickProcess->readAllStandardOutput();
                qDebug() << "Error:" << wgQuickProcess->readAllStandardError();

                deleteLater();

                if (exitStatus != QProcess::NormalExit || exitCode != 0) {
                    emit failed();
                    return;
                }

                emit succeeded();
            });

    wgQuickProcess->start(WG_QUICK, arguments);
}
