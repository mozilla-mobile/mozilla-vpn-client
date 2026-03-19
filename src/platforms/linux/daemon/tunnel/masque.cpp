/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "masque.h"

#include <arpa/inet.h>
#include <linux/fib_rules.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <mntent.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <QCoreApplication>
#include <QEventLoop>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTimer>
#include <chrono>
#include <thread>

#include "daemon/protocols/masque.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/server.h"

namespace {
Logger logger("MasqueTunnelLinux");
Logger logmasque("MasqueDaemon");
}  // namespace

MasqueTunnelLinux::MasqueTunnelLinux(QObject* parent)
    : MasqueTunnel(parent), m_firewall(this), m_daemonProcess(this) {
  MZ_COUNT_CTOR(MasqueTunnelLinux);
  removeInterfaceIfExists();
  // Connect signals to monitor the daemon
  connect(&m_daemonProcess, &QProcess::readyReadStandardOutput, this,
          &MasqueTunnelLinux::daemonStdoutReady);
  connect(&m_daemonProcess, &QProcess::errorOccurred, this,
          &MasqueTunnelLinux::daemonErrorOccurred);
  connect(&m_daemonProcess,
          QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
          &MasqueTunnelLinux::daemonFinished);
  connect(&m_daemonProcess, &QProcess::readyReadStandardError, this,
          &MasqueTunnelLinux::daemonStderrReady);

  logger.debug() << "MasqueTunnelLinux created.";
}

MasqueTunnelLinux::~MasqueTunnelLinux() {
  MZ_COUNT_DTOR(MasqueTunnelLinux);
  removeInterfaceIfExists();
  logger.debug() << "MasqueTunnelLinux destroyed.";
}

bool MasqueTunnelLinux::addInterface(const InterfaceConfig& config) {
  Q_ASSERT(config.m_protocolType == Server::ProtocolType::Masque);

  // Check if daemon is already running
  if (m_daemonProcess.state() != QProcess::NotRunning) {
    logger.warning() << "Unable to start: daemon process already running";
    return false;
  }
  QString programPath = "/usr/bin/masque-vpn";

  // Check if the executable exists and is executable
  QFileInfo programInfo(programPath);
  if (!programInfo.exists()) {
    logger.error() << "MASQUE daemon executable not found at:" << programPath;
    return false;
  }
  if (!programInfo.isExecutable()) {
    logger.error() << "MASQUE daemon is not executable:" << programPath;
    return false;
  }

  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  m_daemonProcess.setProcessEnvironment(pe);
  m_daemonProcess.setProgram(programPath);
  // CRITICAL - This approach is UNSAFE and experiment only
  // command injection can happen here, this should be done via config file,
  // stdin or parameter should be sanitized
  m_daemonProcess.setArguments(
      QStringList({"-verbose", "-token", config.m_privateKey, "-relay",
                   config.m_hostname, "-tun", interfaceName()}));

  // Start the daemon process
  logger.info() << "Starting MASQUE daemon...";
  m_daemonProcess.start();

  // Wait for the process to start
  if (!m_daemonProcess.waitForStarted(5000)) {
    logger.error() << "Failed to start MASQUE daemon. Error:"
                   << m_daemonProcess.errorString()
                   << "Error code:" << m_daemonProcess.error();
    return false;
  }
  if (!waitForInterfaceReady()) {
    logger.error() << "MASQUE interface did not become ready in time.";
    m_daemonProcess.terminate();
    if (!m_daemonProcess.waitForFinished(5000)) {
      m_daemonProcess.kill();
      m_daemonProcess.waitForFinished(5000);
    }
    return false;
  }

  qint64 pid = m_daemonProcess.processId();
  logger.info() << "MASQUE daemon started with PID:" << pid;
  return true;
}

bool MasqueTunnelLinux::deleteInterface() {
  m_connections.clear();
  if (m_daemonProcess.state() == QProcess::NotRunning) {
    logger.debug() << "No daemon process to terminate";
    return false;
  }

  qint64 pid = m_daemonProcess.processId();
  logger.info() << "Terminating MASQUE daemon (PID:" << pid << ")...";

  // Attempt to terminate gracefully with SIGTERM
  m_daemonProcess.terminate();

  // Wait up to 5 seconds for graceful shutdown
  if (!m_daemonProcess.waitForFinished(5000)) {
    logger.warning() << "Daemon did not terminate gracefully, sending SIGKILL";
    // Force kill with SIGKILL
    m_daemonProcess.kill();
    m_daemonProcess.waitForFinished(5000);
  }
  logger.info() << "MASQUE daemon terminated";
  return true;
}

bool MasqueTunnelLinux::interfaceExists() {
  unsigned int ifindex = if_nametoindex(MASQUE_INTERFACE);

  if (ifindex == 0) {
    logger.debug() << "Interface" << MASQUE_INTERFACE << "does not exist";
    return false;
  }

  logger.debug() << "Interface" << MASQUE_INTERFACE << "exists with index"
                 << ifindex;
  return true;
}

bool MasqueTunnelLinux::waitForInterfaceReady() {
  QTimer timeout;
  timeout.setSingleShot(true);
  timeout.start(10000);  // Wait up to 10 seconds for the interface to be ready

  while ((m_daemonProcess.state() == QProcess::Running) && timeout.isActive()) {
    if (interfaceExists()) {
      logger.info() << "Interface" << MASQUE_INTERFACE << "is ready";
      return true;
    }
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
  }

  return false;
}

QJsonObject MasqueTunnelLinux::getStatus() const {
  QJsonObject json;
  if (m_daemonProcess.state() == QProcess::NotRunning) {
    json.insert("connected", QJsonValue(false));
    return json;
  }
  json.insert("connected", QJsonValue(true));

  const ConnectionState& connection = m_connections.first();
  json.insert("serverIpv4Gateway",
              QJsonValue(connection.m_config.m_serverIpv4Gateway));
  json.insert("deviceIpv4Address",
              QJsonValue(connection.m_config.m_deviceIpv4Address));
  json.insert("date", connection.m_date.toString());
  json.insert("txBytes", QJsonValue(0));
  json.insert("rxBytes", QJsonValue(0));
  return json;
}

void MasqueTunnelLinux::daemonStdoutReady() {
  for (;;) {
    QByteArray line = m_daemonProcess.readLine();
    if (line.length() <= 0) {
      break;
    }
    logmasque.debug() << QString::fromUtf8(line);
  }
}

void MasqueTunnelLinux::daemonStderrReady() {
  if (m_daemonProcess.state() == QProcess::NotRunning) {
    return;
  }

  logmasque.debug() << m_daemonProcess.readAllStandardError().trimmed();
}

void MasqueTunnelLinux::daemonErrorOccurred(QProcess::ProcessError error) {
  logger.warning() << "Daemon process encountered an error:" << error;
  emit backendFailure();
}

void MasqueTunnelLinux::daemonFinished(int exitCode,
                                       QProcess::ExitStatus exitStatus) {
  if ((exitStatus != QProcess::NormalExit) || (exitCode != 0)) {
    logger.warning() << "Daemon process exited with code:" << exitCode;
    emit backendFailure();
  } else {
    logger.info() << "Daemon process exited normally";
  }
}
