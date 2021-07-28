/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wireguardutilsmacos.h"
#include "leakdetector.h"
#include "logger.h"

#include <QDir>
#include <QFile>
#include <QTimer>

constexpr const int WG_TUN_PROC_TIMEOUT = 5000;

namespace {
Logger logger(LOG_MACOS, "WireguardUtilsMacos");
Logger logwireguard(LOG_MACOS, "WireguardGo");
};  // namespace

WireguardUtilsMacos::WireguardUtilsMacos(QObject* parent)
    : WireguardUtils(parent), m_tunnel(this) {
  MVPN_COUNT_CTOR(WireguardUtilsMacos);
  logger.log() << "WireguardUtilsMacos created.";

  connect(&m_tunnel, SIGNAL(readyReadStandardOutput()), this,
          SLOT(tunnelStdoutReady()));
}

WireguardUtilsMacos::~WireguardUtilsMacos() {
  MVPN_COUNT_DTOR(WireguardUtilsMacos);
  logger.log() << "WireguardUtilsMacos destroyed.";
}

void WireguardUtilsMacos::tunnelStdoutReady() {
  for (;;) {
    QByteArray line = m_tunnel.readLine();
    if (line.length() <= 0) {
      break;
    }
    logwireguard.log() << QString::fromUtf8(line);
  }
}

bool WireguardUtilsMacos::addInterface(const InterfaceConfig& config) {
  Q_UNUSED(config);
  if (m_tunnel.state() != QProcess::NotRunning) {
    logger.log() << "Unable to start: tunnel process already running";
    return false;
  }

  QString wgNameFile = QString("/var/run/wireguard/%1.name").arg(WG_INTERFACE);
  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  pe.insert("WG_TUN_NAME_FILE", wgNameFile);
#ifdef QT_DEBUG
  pe.insert("LOG_LEVEL", "debug");
#endif
  m_tunnel.setProcessEnvironment(pe);

  QDir appPath(QCoreApplication::applicationDirPath());
  appPath.cdUp();
  appPath.cd("Resources");
  appPath.cd("utils");
  QStringList wgArgs = {"-f", "utun"};
  m_tunnel.start(appPath.filePath("wireguard-go"), wgArgs);
  if (!m_tunnel.waitForStarted(WG_TUN_PROC_TIMEOUT)) {
    logger.log() << "Unable to start tunnel process due to timeout";
    m_tunnel.kill();
    return false;
  }

  m_ifname = waitForTunnelName(wgNameFile);
  if (m_ifname.isNull()) {
    logger.log() << "Unable to read tunnel interface name";
    m_tunnel.kill();
    return false;
  }

  logger.log() << "Created wireguard interface" << m_ifname;
  return true;
}

bool WireguardUtilsMacos::deleteInterface() {
  if (m_tunnel.state() == QProcess::NotRunning) {
    return false;
  }

  // Attempt to terminate gracefully.
  m_tunnel.terminate();
  if (!m_tunnel.waitForFinished(WG_TUN_PROC_TIMEOUT)) {
    m_tunnel.kill();
    m_tunnel.waitForFinished(WG_TUN_PROC_TIMEOUT);
  }

  // Garbage collect.
  QString wgNameFile = QString("/var/run/wireguard/%1.name").arg(WG_INTERFACE);
  QFile::remove(wgNameFile);
  return true;
}

// dummy implementations for now
bool WireguardUtilsMacos::updatePeer(const InterfaceConfig& config) {
  Q_UNUSED(config);
  return true;
}
bool WireguardUtilsMacos::deletePeer(const QString& pubkey) {
  Q_UNUSED(pubkey);
  return true;
}
WireguardUtils::peerStatus WireguardUtilsMacos::getPeerStatus(
    const QString& pubkey) {
  Q_UNUSED(pubkey);
  return peerStatus();
}

bool WireguardUtilsMacos::updateRoutePrefix(const IPAddressRange& prefix,
                                            int hopindex) {
  Q_UNUSED(prefix);
  Q_UNUSED(hopindex);
  return true;
}

bool WireguardUtilsMacos::deleteRoutePrefix(const IPAddressRange& prefix,
                                            int hopindex) {
  Q_UNUSED(prefix);
  Q_UNUSED(hopindex);
  return true;
}

// static
QString WireguardUtilsMacos::waitForTunnelName(const QString& filename) {
  QTimer timeout;
  timeout.setSingleShot(true);
  timeout.start(WG_TUN_PROC_TIMEOUT);

  QFile file(filename);
  while (!file.exists() && timeout.isActive()) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
  }
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return QString();
  }
  return QString::fromLocal8Bit(file.readLine());
}