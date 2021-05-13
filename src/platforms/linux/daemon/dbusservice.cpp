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

namespace {
Logger logger(LOG_LINUX, "DBusService");
}

DBusService::DBusService(QObject* parent) : Daemon(parent) {
  MVPN_COUNT_CTOR(DBusService);
  bool cleanStart = removeInterfaceIfExists();
  if (!removeInterfaceIfExists()) {
    qFatal("Interface `%s` exists and cannot be removed. Cannot proceed!",
           WG_INTERFACE);
  }
}

DBusService::~DBusService() { MVPN_COUNT_DTOR(DBusService); }

WireguardUtils* DBusService::wgutils() {
  if (!m_wgutils) {
    m_wgutils = new WireguardUtilsLinux(this);
  }
  return m_wgutils;
}

IPUtils* DBusService::iputils() {
  if (!m_iputils) {
    m_iputils = new IPUtilsLinux(this);
  }
  return m_iputils;
}

void DBusService::setAdaptor(DbusAdaptor* adaptor) {
  Q_ASSERT(!m_adaptor);
  m_adaptor = adaptor;
}

bool DBusService::removeInterfaceIfExists() {
  if (wgutils()->interfaceExists()) {
    logger.log() << "Device already exists. Let's remove it.";
    if (!wgutils()->deleteInterface()) {
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

  InterfaceConfig config;
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

QString DBusService::status() { return QString(getStatus()); }

QByteArray DBusService::getStatus() {
  logger.log() << "Status request";
  QJsonObject json;
  if (!wgutils()->interfaceExists()) {
    logger.log() << "Unable to get device";
    json.insert("status", QJsonValue(false));
    return QJsonDocument(json).toJson(QJsonDocument::Compact);
  }
  json.insert("status", QJsonValue(true));
  json.insert("serverIpv4Gateway",
              QJsonValue(m_lastConfig.m_serverIpv4Gateway));
  json.insert("deviceIpv4Address",
              QJsonValue(m_lastConfig.m_deviceIpv4Address));
  WireguardUtilsLinux::peerBytes pb = wgutils()->getThroughputForInterface();
  json.insert("txBytes", QJsonValue(pb.txBytes));
  json.insert("rxBytes", QJsonValue(pb.rxBytes));

  return QJsonDocument(json).toJson(QJsonDocument::Compact);
}

QString DBusService::getLogs() {
  logger.log() << "Log request";
  return Daemon::logs();
}

bool DBusService::run(Op op, const InterfaceConfig& config) {
  QStringList addresses;
  for (const IPAddressRange& ip : config.m_allowedIPAddressRanges) {
    addresses.append(ip.toString());
  }

  return WgQuickProcess::run(
      op, config.m_privateKey, config.m_deviceIpv4Address,
      config.m_deviceIpv6Address, config.m_serverIpv4Gateway,
      config.m_serverIpv6Gateway, config.m_serverPublicKey,
      config.m_serverIpv4AddrIn, config.m_serverIpv6AddrIn,
      addresses.join(", "), config.m_serverPort, config.m_ipv6Enabled);
}

bool DBusService::switchServer(const InterfaceConfig& config) {
  logger.log() << "Switching server";
  return wgutils()->configureInterface(config);
}

bool DBusService::supportServerSwitching(const InterfaceConfig& config) const {
  return m_lastConfig.m_privateKey == config.m_privateKey &&
         m_lastConfig.m_deviceIpv4Address == config.m_deviceIpv4Address &&
         m_lastConfig.m_deviceIpv6Address == config.m_deviceIpv6Address &&
         m_lastConfig.m_serverIpv4Gateway == config.m_serverIpv4Gateway &&
         m_lastConfig.m_serverIpv6Gateway == config.m_serverIpv6Gateway;
}
