/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wgquickprocess.h"
#include "../../src/logger.h"

#include <QCoreApplication>
#include <QProcess>

constexpr const char *WG_QUICK = "wg-quick";

namespace {
Logger logger("WgQuickProcess");
}

WgQuickProcess::WgQuickProcess(WgQuickProcess::Op op) : m_op(op) {}

void WgQuickProcess::run(const QString &privateKey,
                         const QString &deviceIpv4Address,
                         const QString &deviceIpv6Address,
                         const QString &serverIpv4Gateway,
                         const QString &serverIpv6Gateway,
                         const QString &serverPublicKey,
                         const QString &serverIpv4AddrIn,
                         const QString &serverIpv6AddrIn,
                         int serverPort,
                         bool ipv6Enabled,
                         bool localNetworkAccess)
{
    Q_UNUSED(serverIpv6AddrIn);

    QByteArray content;
    content.append("[Interface]\nPrivateKey = ");
    content.append(privateKey);
    content.append("\nAddress = ");
    content.append(deviceIpv4Address);

    if (ipv6Enabled) {
        content.append(", ");
        content.append(deviceIpv6Address);
    }

    content.append("\nDNS = ");
    content.append(serverIpv4Gateway);

    if (ipv6Enabled) {
        content.append(", ");
        content.append(serverIpv6Gateway);
    }

    content.append("\n\n[Peer]\nPublicKey = ");
    content.append(serverPublicKey);
    content.append("\nEndpoint = ");
    content.append(serverIpv4AddrIn);
    content.append(QString(":%1").arg(serverPort));

    /* In theory, we should use the ipv6 endpoint, but wireguard doesn't seem
     * to be happy if there are 2 endpoints.
    if (ipv6Enabled) {
        content.append("\nEndpoint = [");
        content.append(serverIpv6AddrIn);
        content.append(QString("]:%1").arg(serverPort));
    }
    */

    content.append("\nAllowedIPs = 0.0.0.0/0");

    if (ipv6Enabled) {
        content.append(",::0");
    }

    if (localNetworkAccess) {
        content.append(",128.0.0.1/1");

        if (ipv6Enabled) {
            content.append(",8000::/1");
        }
    }

    content.append("\n");

    if (!tmpDir.isValid()) {
        qWarning("Cannot create a temporary directory");
        emit failed();
        return;
    }

    QDir dir(tmpDir.path());
    QFile file(dir.filePath(QString("%1.conf").arg(WG_INTERFACE)));
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
        logger.log() << "Error occurred" << error;
        deleteLater();
        emit failed();
    });

    connect(wgQuickProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, wgQuickProcess](int exitCode, QProcess::ExitStatus exitStatus) {
                logger.log() << "Execution finished" << exitCode;

                qWarning("wg-quick stdout:\n%ls\n",
                         qUtf16Printable(wgQuickProcess->readAllStandardOutput()));
                qWarning("wg-quick stderr:\n%ls\n",
                         qUtf16Printable(wgQuickProcess->readAllStandardError()));

                deleteLater();

                if (exitStatus != QProcess::NormalExit || exitCode != 0) {
                    emit failed();
                    return;
                }

                emit succeeded();
            });

    wgQuickProcess->start(WG_QUICK, arguments);
}
