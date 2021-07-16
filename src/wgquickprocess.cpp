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
#if defined(MVPN_MACOS_DAEMON)
    LOG_MACOS
#elif defined(MVPN_WINDOWS)
    LOG_WINDOWS
#endif
    ,
    "WgQuickProcess");
}  // namespace

// static
bool WgQuickProcess::createConfigFile(const QString& outputFile,
                                      const InterfaceConfig& config,
                                      const QMap<QString, QString>& extra) {
#define VALIDATE(x) \
  if (x.contains("\n")) return false;

  VALIDATE(config.m_privateKey);
  VALIDATE(config.m_deviceIpv4Address);
  VALIDATE(config.m_deviceIpv6Address);
  VALIDATE(config.m_serverIpv4Gateway);
  VALIDATE(config.m_serverIpv6Gateway);
  VALIDATE(config.m_serverPublicKey);
  VALIDATE(config.m_serverIpv4AddrIn);
  VALIDATE(config.m_serverIpv6AddrIn);
#undef VALIDATE

  QString content;
  QTextStream out(&content);
  out << "[Interface]\n";
  out << "PrivateKey = " << config.m_privateKey << "\n";

  QStringList addresses(config.m_deviceIpv4Address);
  QStringList dnsServers(config.m_dnsServer);
  if (config.m_ipv6Enabled) {
    addresses.append(config.m_deviceIpv6Address);
    // If the DNS is not the Gateway, it's a user defined DNS
    // thus, not add any other :)
    if (config.m_dnsServer == config.m_serverIpv4Gateway) {
      dnsServers.append(config.m_serverIpv6Gateway);
    }
  }
  out << "Address = " << addresses.join(", ") << "\n";
  out << "DNS = " << dnsServers.join(", ") << "\n";

  // If any extra config was provided, append it now.
  for (const QString& key : extra.keys()) {
    out << key << " = " << extra[key] << "\n";
  }

  out << "\n[Peer]\n";
  out << "PublicKey = " << config.m_serverPublicKey << "\n";
  out << "Endpoint = " << config.m_serverIpv4AddrIn.toUtf8() << ":"
      << config.m_serverPort << "\n";

  /* In theory, we should use the ipv6 endpoint, but wireguard doesn't seem
   * to be happy if there are 2 endpoints.
  if (ipv6Enabled) {
    out << "Endpoint = [" << config.m_serverIpv6AddrIn << "]:"
        << config.m_serverPort << "\n";
  }
  */
  QStringList ranges;
  for (const IPAddressRange& ip : config.m_allowedIPAddressRanges) {
    ranges.append(ip.toString());
  }
  out << "AllowedIPs = " << ranges.join(", ") << "\n";

#ifdef QT_DEBUG
  logger.log() << content;
#endif

  QFile file(outputFile);
  if (!file.open(QIODevice::WriteOnly)) {
    qWarning("Unable to create a file in the temporary folder");
    return false;
  }

  qint64 written = file.write(content.toUtf8());
  if (written != content.length()) {
    qWarning("Unable to write the whole configuration file");
    return false;
  }

  file.close();
  return true;
}

// static
bool WgQuickProcess::createConfigFile(
    const QString& outputFile, const QString& privateKey,
    const QString& deviceIpv4Address, const QString& deviceIpv6Address,
    const QString& serverIpv4Gateway, const QString& serverIpv6Gateway,
    const QString& serverPublicKey, const QString& serverIpv4AddrIn,
    const QString& serverIpv6AddrIn, const QString& allowedIPAddressRanges,
    int serverPort, bool ipv6Enabled, const QString& dnsServer) {
  Q_UNUSED(serverIpv6AddrIn);

  InterfaceConfig config;
  config.m_privateKey = privateKey;
  config.m_deviceIpv4Address = deviceIpv4Address;
  config.m_deviceIpv6Address = deviceIpv6Address;
  config.m_serverIpv4Gateway = serverIpv4Gateway;
  config.m_serverIpv6Gateway = serverIpv6Gateway;
  config.m_serverPublicKey = serverPublicKey;
  config.m_serverIpv4AddrIn = serverIpv4AddrIn;
  config.m_serverIpv6AddrIn = serverIpv6AddrIn;
  config.m_dnsServer = dnsServer;
  config.m_serverPort = serverPort;
  config.m_ipv6Enabled = ipv6Enabled;

  for (const QString& range : allowedIPAddressRanges.split(',')) {
    config.m_allowedIPAddressRanges.append(IPAddressRange(range));
  }

  return createConfigFile(outputFile, config);
}

// static
bool WgQuickProcess::run(
    Daemon::Op op, const QString& privateKey, const QString& deviceIpv4Address,
    const QString& deviceIpv6Address, const QString& serverIpv4Gateway,
    const QString& serverIpv6Gateway, const QString& serverPublicKey,
    const QString& serverIpv4AddrIn, const QString& serverIpv6AddrIn,
    const QString& allowedIPAddressRanges, int serverPort, bool ipv6Enabled,
    const QString& dnsServer) {
  QTemporaryDir tmpDir;
  if (!tmpDir.isValid()) {
    logger.log() << "Cannot create a temporary directory"
                 << tmpDir.errorString();
    return false;
  }

  QDir dir(tmpDir.path());
  QString configFile(dir.filePath(QString("%1.conf").arg(WG_INTERFACE)));

  if (!createConfigFile(configFile, privateKey, deviceIpv4Address,
                        deviceIpv6Address, serverIpv4Gateway, serverIpv6Gateway,
                        serverPublicKey, serverIpv4AddrIn, serverIpv6AddrIn,
                        allowedIPAddressRanges, serverPort, ipv6Enabled,
                        dnsServer)) {
    logger.log() << "Failed to create the config file";
    return false;
  }

  QStringList arguments;
  arguments.append(op == Daemon::Up ? "up" : "down");
  arguments.append(configFile);

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

// static
QString WgQuickProcess::scriptPath() {
#if defined(MVPN_MACOS_DAEMON)
  QDir appPath(QCoreApplication::applicationDirPath());
  appPath.cdUp();
  appPath.cd("Resources");
  appPath.cd("utils");
  return appPath.filePath("helper.sh");
#endif
  return QString();
}
