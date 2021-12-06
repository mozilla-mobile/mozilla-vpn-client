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

  QStringList addresses;
  if (!config.m_deviceIpv4Address.isNull()) {
    addresses.append(config.m_deviceIpv4Address);
  }
  if (!config.m_deviceIpv6Address.isNull()) {
    addresses.append(config.m_deviceIpv6Address);
  }
  if (addresses.isEmpty()) {
    logger.error() << "Failed to create WG quick config with no addresses";
    return false;
  }
  out << "Address = " << addresses.join(", ") << "\n";

  if (!config.m_dnsServer.isNull()) {
    QStringList dnsServers(config.m_dnsServer);
    // If the DNS is not the Gateway, it's a user defined DNS
    // thus, not add any other :)
    if (config.m_dnsServer == config.m_serverIpv4Gateway) {
      dnsServers.append(config.m_serverIpv6Gateway);
    }
    out << "DNS = " << dnsServers.join(", ") << "\n";
  }

  // If any extra config was provided, append it now.
  for (const QString& key : extra.keys()) {
    out << key << " = " << extra[key] << "\n";
  }

  // For windows, we don't want to include the peer configuration since they
  // will be passed later as a UAPI command. and we need to fiddle with the
  // interface in between creation and peer bringup.
  //
  // This function should go away as soon as we fixup the Mac implementation
  // anyways.
#if !defined(MVPN_WINDOWS) && !defined(MVPN_MACOS_DAEMON)
  out << "\n[Peer]\n";
  out << "PublicKey = " << config.m_serverPublicKey << "\n";
  out << "Endpoint = " << config.m_serverIpv4AddrIn.toUtf8() << ":"
      << config.m_serverPort << "\n";

  /* In theory, we should use the ipv6 endpoint, but wireguard doesn't seem
   * to be happy if there are 2 endpoints.
  out << "Endpoint = [" << config.m_serverIpv6AddrIn << "]:"
      << config.m_serverPort << "\n";
  */
  QStringList ranges;
  for (const IPAddress& ip : config.m_allowedIPAddressRanges) {
    ranges.append(ip.toString());
  }
  out << "AllowedIPs = " << ranges.join(", ") << "\n";
#endif

#ifdef MVPN_DEBUG
  logger.debug() << content;
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
    int serverPort, const QString& dnsServer) {
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

  for (const QString& range : allowedIPAddressRanges.split(',')) {
    config.m_allowedIPAddressRanges.append(IPAddress(range));
  }

  return createConfigFile(outputFile, config);
}
