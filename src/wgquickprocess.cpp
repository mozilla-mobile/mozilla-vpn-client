/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wgquickprocess.h"
#include "../../src/logger.h"

#include <QCoreApplication>
#include <QTemporaryDir>
#include <QProcess>

namespace {
Logger logger(
#if defined(MVPN_LINUX)
    LOG_LINUX
#elif defined(MVPN_MACOS_DAEMON)
    LOG_MACOS
#endif
    ,
    "WgQuickProcess");

QString scriptPath() {
  QDir appPath(QCoreApplication::applicationDirPath());
#if defined(MVPN_LINUX)
  appPath.cdUp();
  appPath.cd("share");
  return appPath.filePath("mozillavpn_wghelper.sh");
  ;
#elif defined(MVPN_MACOS_DAEMON)
  appPath.cdUp();
  appPath.cd("Resources");
  appPath.cd("utils");
  return appPath.filePath("helper.sh");
#else
#  error Unsupported platform
#endif
}

}  // namespace

bool WgQuickProcess::validateWgArgs(
    const QString& privateKey, const QString& deviceIpv4Address,
    const QString& deviceIpv6Address, const QString& serverIpv4Gateway,
    const QString& serverIpv6Gateway, const QString& serverPublicKey,
    const QString& serverIpv4AddrIn, const QString& serverIpv6AddrIn,
    const QString& allowedIPAddressRanges, int serverPort, bool ipv6Enabled) {
  // We may want to do more advanced validation in the future
  Q_UNUSED(serverPort);
  Q_UNUSED(ipv6Enabled);

#define VALIDATE(x) \
  if (x.contains("\n")) return false;
  VALIDATE(privateKey);
  VALIDATE(deviceIpv4Address);
  VALIDATE(deviceIpv6Address);
  VALIDATE(serverIpv4Gateway);
  VALIDATE(serverIpv6Gateway);
  VALIDATE(serverPublicKey);
  VALIDATE(serverIpv4AddrIn);
  VALIDATE(serverIpv6AddrIn);
  VALIDATE(allowedIPAddressRanges);
#undef VALIDATE

  return true;
}

QString WgQuickProcess::writeWgConfigFile(
    QTemporaryDir& tmpDir, const QString& privateKey,
    const QString& deviceIpv4Address, const QString& deviceIpv6Address,
    const QString& serverIpv4Gateway, const QString& serverIpv6Gateway,
    const QString& serverPublicKey, const QString& serverIpv4AddrIn,
    const QString& serverIpv6AddrIn, const QString& allowedIPAddressRanges,
    int serverPort, bool ipv6Enabled) {
  Q_UNUSED(serverIpv6AddrIn);

  QByteArray content;
  content.append("[Interface]\nPrivateKey = ");
  content.append(privateKey.toUtf8());
  content.append("\nAddress = ");
  content.append(deviceIpv4Address.toUtf8());

  if (ipv6Enabled) {
    content.append(", ");
    content.append(deviceIpv6Address.toUtf8());
  }

  content.append("\nDNS = ");
  content.append(serverIpv4Gateway.toUtf8());

  if (ipv6Enabled) {
    content.append(", ");
    content.append(serverIpv6Gateway.toUtf8());
  }

  content.append("\n\n[Peer]\nPublicKey = ");
  content.append(serverPublicKey.toUtf8());
  content.append("\nEndpoint = ");
  content.append(serverIpv4AddrIn.toUtf8());
  content.append(QString(":%1").arg(serverPort).toUtf8());

  /* In theory, we should use the ipv6 endpoint, but wireguard doesn't seem
   * to be happy if there are 2 endpoints.
  if (ipv6Enabled) {
      content.append("\nEndpoint = [");
      content.append(serverIpv6AddrIn);
      content.append(QString("]:%1").arg(serverPort));
  }
  */

  content.append(
      QString("\nAllowedIPs = %1\n").arg(allowedIPAddressRanges).toUtf8());

  if (!tmpDir.isValid()) {
    qWarning("Cannot create a temporary directory");
  }
  QDir dir(tmpDir.path());
  QFile file(dir.filePath(QString("%1.conf").arg(WG_INTERFACE)));

  if (!file.open(QIODevice::ReadWrite)) {
    qWarning("Unable to create a file in the temporary folder");
    return "";
  }

  qint64 written = file.write(content);

  if (written != content.length()) {
    qWarning("Unable to write the whole configuration file");
    return "";
  }

  return file.fileName();
}

// static
bool WgQuickProcess::run(
    Daemon::Op op, const QString& privateKey, const QString& deviceIpv4Address,
    const QString& deviceIpv6Address, const QString& serverIpv4Gateway,
    const QString& serverIpv6Gateway, const QString& serverPublicKey,
    const QString& serverIpv4AddrIn, const QString& serverIpv6AddrIn,
    const QString& allowedIPAddressRanges, int serverPort, bool ipv6Enabled) {
  if (!validateWgArgs(privateKey, deviceIpv4Address, deviceIpv6Address,
                      serverIpv4Gateway, serverIpv6Gateway, serverPublicKey,
                      serverIpv4AddrIn, serverIpv6AddrIn,
                      allowedIPAddressRanges, serverPort, ipv6Enabled)) {
    qWarning("Config arguments are not valid.");
    return false;
  }

  QTemporaryDir tmpDir;
  QString confFile = writeWgConfigFile(
      tmpDir, privateKey, deviceIpv4Address, deviceIpv6Address,
      serverIpv4Gateway, serverIpv6Gateway, serverPublicKey, serverIpv4AddrIn,
      serverIpv6AddrIn, allowedIPAddressRanges, serverPort, ipv6Enabled);

  if (confFile.isEmpty()) {
    return false;
  }

  QStringList arguments;
  arguments.append(op == Daemon::Up ? "up" : "down");
  arguments.append(confFile);

  QString app = scriptPath();
  logger.log() << "Start:" << app << " - arguments:" << arguments;

  QProcess wgQuickProcess;
  wgQuickProcess.start(app, arguments);

  if (!wgQuickProcess.waitForFinished(-1)) {
    logger.log() << "Error occurred:" << wgQuickProcess.errorString();
    return false;
  }

  logger.log() << "Execution finished" << wgQuickProcess.exitCode();

  logger.log() << "wg-quick stdout:" << Qt::endl
               << qUtf8Printable(wgQuickProcess.readAllStandardOutput())
               << Qt::endl;
  logger.log() << "wg-quick stderr:" << Qt::endl
               << qUtf8Printable(wgQuickProcess.readAllStandardError())
               << Qt::endl;

  return wgQuickProcess.exitCode() == 0;
}
