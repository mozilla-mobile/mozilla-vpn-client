/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wireguardutilsmacos.h"
#include "leakdetector.h"
#include "logger.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QLocalSocket>
#include <QTimer>

#include <errno.h>

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

  // Send a UAPI command to configure the interface
  QString message("set=1\n");
  QByteArray privateKey = QByteArray::fromBase64(config.m_privateKey.toUtf8());
  QTextStream out(&message);
  out << "private_key=" << QString(privateKey.toHex()) << "\n";
  out << "replace_peers=true\n";
  int err = uapiErrno(uapiCommand(message));
  if (err != 0) {
    logger.log() << "Interface configuration failed:" << strerror(err);
  }
  return (err == 0);
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
  QByteArray publicKey =
      QByteArray::fromBase64(qPrintable(config.m_serverPublicKey));

  // Update/create the peer config
  QString message;
  QTextStream out(&message);
  out << "set=1\n";
  out << "public_key=" << QString(publicKey.toHex()) << "\n";
  if (!config.m_serverIpv4AddrIn.isNull()) {
    out << "endpoint=" << config.m_serverIpv4AddrIn << ":";
  } else if (!config.m_serverIpv6AddrIn.isNull()) {
    out << "endpoint=[" << config.m_serverIpv6AddrIn << "]:";
  } else {
    logger.log() << "Failed to create peer with no endpoints";
    return false;
  }
  out << config.m_serverPort << "\n";

  out << "replace_allowed_ips=true\n";
  for (const IPAddressRange& ip : config.m_allowedIPAddressRanges) {
    out << "allowed_ip=" << ip.toString() << "\n";
  }

  int err = uapiErrno(uapiCommand(message));
  if (err != 0) {
    logger.log() << "Peer configuration failed:" << strerror(err);
  }
  return (err == 0);
}

bool WireguardUtilsMacos::deletePeer(const QString& pubkey) {
  QByteArray publicKey = QByteArray::fromBase64(qPrintable(pubkey));

  QString message;
  QTextStream out(&message);
  out << "set=1\n";
  out << "public_key=" << QString(publicKey.toHex()) << "\n";
  out << "remove=true\n";

  int err = uapiErrno(uapiCommand(message));
  if (err != 0) {
    logger.log() << "Peer deletion failed:" << strerror(err);
  }
  return (err == 0);
}

WireguardUtils::peerStatus WireguardUtilsMacos::getPeerStatus(
    const QString& pubkey) {
  peerStatus status = {0, 0};
  QString hexkey = QByteArray::fromBase64(pubkey.toUtf8()).toHex();
  QString reply = uapiCommand("get=1");
  bool match = false;

  logger.log() << "Getting status from getPeerStatus()";

  for (const QString& line : reply.split('\n')) {
    int eq = line.indexOf('=');
    if (eq <= 0) {
      continue;
    }
    QString name = line.left(eq);
    QString value = line.mid(eq + 1);

    if (name == "public_key") {
      match = (value == hexkey);
      continue;
    } else if (!match) {
      continue;
    }

    if (name == "tx_bytes") {
      status.txBytes = value.toDouble();
    }
    if (name == "rx_bytes") {
      status.rxBytes = value.toDouble();
    }
  }

  return status;
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

QString WireguardUtilsMacos::uapiCommand(const QString& command) {
  QLocalSocket socket;
  QString wgSocketFile = QString("/var/run/wireguard/%1.sock").arg(m_ifname);
  socket.connectToServer(wgSocketFile, QIODevice::ReadWrite);
  if (!socket.waitForConnected(WG_TUN_PROC_TIMEOUT)) {
    logger.log() << "QLocalSocket::waitForConnected() failed:"
                 << socket.errorString();
    return QString();
  }

  // Send the message to the UAPI socket.
  QByteArray message = command.toLocal8Bit();
  while (!message.endsWith("\n\n")) {
    message.append('\n');
  }
  socket.write(message);
  if (!socket.waitForBytesWritten(WG_TUN_PROC_TIMEOUT)) {
    logger.log() << "QLocalSocket::waitForBytesWritten() failed";
    return QString();
  }

  QByteArray reply;
  while (!reply.contains("\n\n")) {
    if (!socket.waitForReadyRead(WG_TUN_PROC_TIMEOUT)) {
      logger.log() << "QLocalSocket::waitForReadyRead() failed";
      return QString();
    }
    reply.append(socket.readAll());
  }
  return QString::fromUtf8(reply).trimmed();
}

// static
int WireguardUtilsMacos::uapiErrno(const QString& reply) {
  for (const QString& line : reply.split("\n")) {
    int eq = line.indexOf('=');
    if (eq <= 0) {
      continue;
    }
    if (line.left(eq) == "errno") {
      return line.mid(eq + 1).toInt();
    }
  }
  return EINVAL;
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
  return QString::fromLocal8Bit(file.readLine()).trimmed();
}