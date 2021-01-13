/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dbusservice.h"
#include "dbus_adaptor.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"
#include "polkithelper.h"
#include "wgquickprocess.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

#if defined(__cplusplus)
extern "C" {
#endif

#include "../../3rdparty/wireguard-tools/contrib/embeddable-wg-library/wireguard.h"

#if defined(__cplusplus)
}
#endif

namespace {
Logger logger(LOG_LINUX, "DBusService");
}

DBusService::DBusService(QObject* parent) : Daemon(parent) {
  MVPN_COUNT_CTOR(DBusService);
}

DBusService::~DBusService() { MVPN_COUNT_DTOR(DBusService); }

void DBusService::setAdaptor(DbusAdaptor* adaptor) {
  Q_ASSERT(!m_adaptor);
  m_adaptor = adaptor;
}

bool DBusService::checkInterface() {
  logger.log() << "Checking interface";

  wg_device* device = nullptr;
  if (wg_get_device(&device, WG_INTERFACE) == 0) {
    logger.log() << "Device already exists. Let's remove it.";
    wg_free_device(device);

    if (wg_del_device(WG_INTERFACE) != 0) {
      logger.log() << "Failed to remove the device.";
      return false;
    }
  }

  return true;
}

QString DBusService::version() {
  logger.log() << "Version request";
  return PROTOCOL_VERSION;
}

bool DBusService::activate(const QString& jsonConfig) {
  logger.log() << "Activate";

  if (!PolkitHelper::instance()->checkAuthorization(
          "org.mozilla.vpn.activate")) {
    logger.log() << "Polkit rejected";
    return false;
  }

  QJsonDocument json = QJsonDocument::fromJson(jsonConfig.toLocal8Bit());
  if (!json.isObject()) {
    logger.log() << "Invalid input";
    return false;
  }

  QJsonObject obj = json.object();

  Config config;
  if (!parseConfig(obj, config)) {
    logger.log() << "Invalid configuration";
    return false;
  }

  return Daemon::activate(config);
}

bool DBusService::deactivate(bool emitSignals) {
  logger.log() << "Deactivate";
  return Daemon::deactivate(emitSignals);
}

QString DBusService::status() {
  logger.log() << "Status request";

  QJsonObject json;

  wg_device* device = nullptr;
  if (wg_get_device(&device, WG_INTERFACE) != 0) {
    logger.log() << "Unable to get device";
    json.insert("status", QJsonValue(false));
    return QJsonDocument(json).toJson(QJsonDocument::Compact);
  }

  uint64_t txBytes = 0;
  uint64_t rxBytes = 0;

  wg_peer* peer;
  wg_for_each_peer(device, peer) {
    txBytes += peer->tx_bytes;
    rxBytes += peer->rx_bytes;
  }

  wg_free_device(device);

  json.insert("status", QJsonValue(true));
  json.insert("serverIpv4Gateway",
              QJsonValue(m_lastConfig.m_serverIpv4Gateway));
  json.insert("txBytes", QJsonValue(double(txBytes)));
  json.insert("rxBytes", QJsonValue(double(rxBytes)));

  return QJsonDocument(json).toJson(QJsonDocument::Compact);
}

QString DBusService::getLogs() {
  logger.log() << "Log request";
  return Daemon::logs();
}

bool DBusService::run(Op op, const Config& config) {

  // Make the config file

  QTemporaryDir tmpDir;

  QString confFile = WgQuickProcess::writeWgConfigFile(
    tmpDir,
    config.m_privateKey, config.m_deviceIpv4Address,
    config.m_deviceIpv6Address, config.m_serverIpv4Gateway,
    config.m_serverIpv6Gateway, config.m_serverPublicKey,
    config.m_serverIpv4AddrIn, config.m_serverIpv6AddrIn,
    config.m_allowedIPAddressRanges.join(", "), config.m_serverPort,
    config.m_ipv6Enabled);

  if (confFile == WgQuickProcess::FAIL) {
    return false;
  }

  // Make our copy of the bash script

  // Read in bash file and write to tmp so can execute it.
  // TODO: This seems cumbersome, but I didn't know a better way to do it

  QFile infile(":/platforms/linux/daemon/linux.bash");
  if (!infile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }
  QByteArray linuxBash = infile.readAll();

  QDir dir(tmpDir.path());
  QFile outfile(dir.filePath(QString("linux.bash")));
  if (!outfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return false;
  }  
  logger.log() << "Writing linux bash script to: " << outfile.fileName();
  outfile.write(linuxBash);

  // Run it

  QStringList arguments;
  arguments.append(outfile.fileName());
  arguments.append(op == Daemon::Up ? "up" : "down");
  arguments.append(confFile);

  logger.log() << "Start our copy of wg-quick script with arguments:" << arguments;

  QProcess p;
  p.start("/bin/bash", arguments);
 
  if (!p.waitForFinished(-1)) {
    logger.log() << "Error occurred:" << p.errorString();
    return false;
  }

  logger.log() << "Execution finished" << p.exitCode();

  logger.log() << "wg-quick stdout:" << Qt::endl
               << qUtf8Printable(p.readAllStandardOutput())
               << Qt::endl;
  logger.log() << "wg-quick stderr:" << Qt::endl
               << qUtf8Printable(p.readAllStandardError())
               << Qt::endl;

  return p.exitCode() == 0;
}
