/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WGQUICKPROCESS_H
#define WGQUICKPROCESS_H

#include "daemon/daemon.h"
#include "daemon/interfaceconfig.h"

#include <QObject>

class WgQuickProcess final {
  Q_DISABLE_COPY_MOVE(WgQuickProcess)

 public:
  static bool run(
      Daemon::Op op, const QString& privateKey,
      const QString& deviceIpv4Address, const QString& deviceIpv6Address,
      const QString& serverIpv4Gateway, const QString& serverIpv6Gateway,
      const QString& serverPublicKey, const QString& serverIpv4AddrIn,
      const QString& serverIpv6AddrIn, const QString& allowedIPAddressRanges,
      int serverPort, bool ipv6Enabled, const QString& dnsServer);

  static bool createConfigFile(
      const QString& configFile, const QString& privateKey,
      const QString& deviceIpv4Address, const QString& deviceIpv6Address,
      const QString& serverIpv4Gateway, const QString& serverIpv6Gateway,
      const QString& serverPublicKey, const QString& serverIpv4AddrIn,
      const QString& serverIpv6AddrIn, const QString& allowedIPAddressRanges,
      int serverPort, bool ipv6Enabled, const QString& dnsServer, bool killswitchDisabled);

  static bool createConfigFile(
      const QString& outputFile, const InterfaceConfig& config,
      const QMap<QString, QString>& extra = QMap<QString, QString>());

  static QString scriptPath();
};

#endif  // WGQUICKPROCESS_H
