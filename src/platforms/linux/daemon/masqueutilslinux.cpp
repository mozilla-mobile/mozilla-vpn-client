
#include "masqueutilslinux.h"

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

#include <QFileInfo>
#include <QProcess>
#include <chrono>
#include <thread>

#include "leakdetector.h"
#include "logger.h"
#include "models/server.h"

namespace {
Logger logger("MasqueUtilsLinux");
Logger logmasque("MasqueDaemon");
}  // namespace

MasqueUtilsLinux::MasqueUtilsLinux(QObject* parent)
    : MasqueUtils(parent), m_firewall(this), m_daemonProcess(this) {
  MZ_COUNT_CTOR(MasqueUtilsLinux);
  // Connect signals to monitor the daemon
  connect(&m_daemonProcess, &QProcess::readyReadStandardOutput, this,
          &MasqueUtilsLinux::daemonStdoutReady);
  connect(&m_daemonProcess, &QProcess::errorOccurred, this,
          &MasqueUtilsLinux::daemonErrorOccurred);
  connect(&m_daemonProcess,
          QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
          &MasqueUtilsLinux::daemonFinished);
  connect(&m_daemonProcess, &QProcess::readyReadStandardError, this,
          &MasqueUtilsLinux::daemonStderrReady);

  logger.debug() << "MasqueUtilsLinux created.";
}

MasqueUtilsLinux::~MasqueUtilsLinux() {
  MZ_COUNT_DTOR(MasqueUtilsLinux);
  // Clean up daemon process if still running
  if (m_daemonProcess.state() != QProcess::NotRunning) {
    m_daemonProcess.terminate();
    if (!m_daemonProcess.waitForFinished(5000)) {
      m_daemonProcess.kill();
      m_daemonProcess.waitForFinished(5000);
    }
  }
  logger.debug() << "MasqueUtilsLinux destroyed.";
}

bool MasqueUtilsLinux::addInterface(const InterfaceConfig& config) {
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
  m_daemonProcess.setArguments(
      QStringList({"-verbose", "-token", "d", "-insecure", "-relay",
                   config.m_serverIpv4AddrIn}));

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

  qint64 pid = m_daemonProcess.processId();
  logger.info() << "MASQUE daemon started with PID:" << pid;

  return true;
}

bool MasqueUtilsLinux::deleteInterface() {
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

bool MasqueUtilsLinux::interfaceExists() {
  unsigned int ifindex = if_nametoindex(MASQUE_INTERFACE);

  if (ifindex == 0) {
    logger.debug() << "Interface" << MASQUE_INTERFACE << "does not exist";
    return false;
  }

  logger.debug() << "Interface" << MASQUE_INTERFACE << "exists with index"
                 << ifindex;
  return true;
}

void MasqueUtilsLinux::daemonStdoutReady() {
  for (;;) {
    QByteArray line = m_daemonProcess.readLine();
    if (line.length() <= 0) {
      break;
    }
    logmasque.debug() << QString::fromUtf8(line);
  }
}

void MasqueUtilsLinux::daemonStderrReady() {
  if (m_daemonProcess.state() == QProcess::NotRunning) {
    return;
  }

  logmasque.debug() << m_daemonProcess.readAllStandardError().trimmed();
}

void MasqueUtilsLinux::daemonErrorOccurred(QProcess::ProcessError error) {
  logger.warning() << "Daemon process encountered an error:" << error;
  emit backendFailure();
}

void MasqueUtilsLinux::daemonFinished(int exitCode,
                                      QProcess::ExitStatus exitStatus) {
  if ((exitStatus != QProcess::NormalExit) || (exitCode != 0)) {
    logger.warning() << "Daemon process exited with code:" << exitCode;
    emit backendFailure();
  } else {
    logger.info() << "Daemon process exited normally";
  }
}
