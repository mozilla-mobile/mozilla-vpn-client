/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosdaemon.h"
#include "leakdetector.h"
#include "logger.h"
#include "wgquickprocess.h"

#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLocalSocket>
#include <QProcess>
#include <QSettings>
#include <QTextStream>
#include <QtGlobal>

constexpr const char* LATEST_IPV4GATEWAY = "latestServerIpv4Gateway";
constexpr const char* LATEST_IPV6GATEWAY = "latestServerIpv6Gateway";
constexpr const char* LATEST_IPV6ENABLED = "latestIpv6Enabled";

namespace {
Logger logger(LOG_MACOS, "MacOSDaemon");
MacOSDaemon* s_daemon = nullptr;
}  // namespace

MacOSDaemon::MacOSDaemon() : Daemon(nullptr) {
  MVPN_COUNT_CTOR(MacOSDaemon);

  logger.debug() << "Daemon created";

  Q_ASSERT(s_daemon == nullptr);
  s_daemon = this;
}

MacOSDaemon::~MacOSDaemon() {
  MVPN_COUNT_DTOR(MacOSDaemon);

  logger.debug() << "Daemon released";

  Q_ASSERT(s_daemon == this);
  s_daemon = nullptr;
}

// static
MacOSDaemon* MacOSDaemon::instance() {
  Q_ASSERT(s_daemon);
  return s_daemon;
}

QByteArray MacOSDaemon::getStatus() {
  logger.debug() << "Status request";

  QJsonObject obj;
  obj.insert("type", "status");
  obj.insert("connected", m_connected);

  if (m_connected) {
    uint64_t txBytes = 0;
    uint64_t rxBytes = 0;

    QDir appPath(QCoreApplication::applicationDirPath());
    appPath.cdUp();
    appPath.cd("Resources");
    appPath.cd("utils");
    QString wgPath = appPath.filePath("wg");

    QStringList arguments{"show", "all", "transfer"};
    logger.debug() << "Start:" << wgPath << " - arguments:" << arguments;

    QProcess wgProcess;
    wgProcess.start(wgPath, arguments);

    if (!wgProcess.waitForFinished(-1)) {
      logger.error() << "Error occurred" << wgProcess.errorString();
    } else {
      QByteArray output = wgProcess.readAllStandardOutput();

      logger.debug() << "wg-quick stdout:" << Qt::endl
                     << qUtf8Printable(output) << Qt::endl;
      logger.debug() << "wg-quick stderr:" << Qt::endl
                     << qUtf8Printable(wgProcess.readAllStandardError())
                     << Qt::endl;

      QStringList lines = QString(output).split("\n");
      for (const QString& line : lines) {
        QStringList parts = line.split("\t");

        if (parts.length() == 4) {
          rxBytes = parts[2].toLongLong();
          txBytes = parts[3].toLongLong();
        }
      }
    }

    obj.insert("status", true);
    obj.insert("serverIpv4Gateway", m_lastConfig.m_serverIpv4Gateway);
    obj.insert("deviceIpv4Address", m_lastConfig.m_deviceIpv4Address);
    obj.insert("date", m_connectionDate.toString());

    obj.insert("txBytes", QJsonValue(double(txBytes)));
    obj.insert("rxBytes", QJsonValue(double(rxBytes)));
  }

  return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

bool MacOSDaemon::run(Daemon::Op op, const InterfaceConfig& config) {
  QStringList addresses;
  for (const IPAddressRange& ip : config.m_allowedIPAddressRanges) {
    addresses.append(ip.toString());
  }

  QSettings settings;

  if (op == Daemon::Up) {
    settings.setValue(LATEST_IPV4GATEWAY, config.m_serverIpv4Gateway);
    settings.setValue(LATEST_IPV6GATEWAY, config.m_serverIpv6Gateway);
    settings.setValue(LATEST_IPV6ENABLED, config.m_ipv6Enabled);
  } else {
    settings.remove(LATEST_IPV4GATEWAY);
    settings.remove(LATEST_IPV6GATEWAY);
    settings.remove(LATEST_IPV6ENABLED);
  }

  return WgQuickProcess::run(
      op, config.m_privateKey, config.m_deviceIpv4Address,
      config.m_deviceIpv6Address, config.m_serverIpv4Gateway,
      config.m_serverIpv6Gateway, config.m_serverPublicKey,
      config.m_serverIpv4AddrIn, config.m_serverIpv6AddrIn,
      addresses.join(", "), config.m_serverPort, config.m_ipv6Enabled,
      config.m_dnsServer);
}

void MacOSDaemon::maybeCleanup() {
  logger.debug() << "Cleanup";

  QString app = WgQuickProcess::scriptPath();

  QSettings settings;
  if (!settings.contains(LATEST_IPV4GATEWAY)) {
    return;
  }

  QString serverIpv4Gateway = settings.value(LATEST_IPV4GATEWAY).toString();
  QString serverIpv6Gateway = settings.value(LATEST_IPV6GATEWAY).toString();
  bool ipv6Enabled = settings.value(LATEST_IPV6ENABLED).toBool();

  QStringList arguments;
  arguments.append("cleanup");
  arguments.append(WG_INTERFACE);
  arguments.append(serverIpv4Gateway.toUtf8());

  if (ipv6Enabled) {
    arguments.append(serverIpv6Gateway.toUtf8());
  }

  logger.debug() << "Start:" << app << " - arguments:" << arguments;

  QProcess wgQuickProcess;
  wgQuickProcess.start(app, arguments);

  if (!wgQuickProcess.waitForFinished(-1)) {
    logger.error() << "Error occurred:" << wgQuickProcess.errorString();
    return;
  }

  logger.debug() << "Execution finished" << wgQuickProcess.exitCode();

  logger.debug() << "wg-quick stdout:" << Qt::endl
                 << qUtf8Printable(wgQuickProcess.readAllStandardOutput())
                 << Qt::endl;
  logger.debug() << "wg-quick stderr:" << Qt::endl
                 << qUtf8Printable(wgQuickProcess.readAllStandardError())
                 << Qt::endl;
}
