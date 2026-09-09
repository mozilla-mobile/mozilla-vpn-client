/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "qprocessobfuscator.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

#include "leakdetector.h"
#include "logger.h"

constexpr int OBFUSCATOR_PROC_TIMEOUT_MS = 5000;
// SO_MARK set is not available in the Flatpak sandbox, so we only use it on
// non-Flatpak Linux builds.
#if defined(MZ_LINUX) && !defined(MZ_FLATPAK)
constexpr uint32_t WG_FIREWALL_MARK = 0xca6c;
#endif

namespace {
Logger logger("QProcessObfuscator");
}

QProcessObfuscator::QProcessObfuscator(const InterfaceConfig& config)
    : m_config(config) {
  MZ_COUNT_CTOR(QProcessObfuscator);

  QString binaryFile = binaryName();
  if (binaryFile.isEmpty()) {
    return;
  }

  binaryFile =
      QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(binaryFile);
  if (!QFileInfo::exists(binaryFile)) {
    logger.error() << "Obfuscator binary not found at" << binaryFile;
    return;
  }
  m_binaryFile = binaryFile;

  // Merge stderr into stdout so we can read the "listening on" announce line
  m_process.setProcessChannelMode(QProcess::MergedChannels);
}

bool QProcessObfuscator::start() { return launch(0); }

bool QProcessObfuscator::restart() {
  stop();
  // Reuse the port assigned by the previous launch so the WireGuard peer config
  // that still points at 127.0.0.1:<port> keeps working.
  return launch(m_localPort);
}

bool QProcessObfuscator::isRunning() const {
  return m_process.state() == QProcess::Running;
}

bool QProcessObfuscator::launch(quint16 listenPort) {
  if (m_binaryFile.isEmpty()) {
    return false;
  }

  const QStringList args = buildArgs(m_config, listenPort);
  if (args.isEmpty()) {
    logger.error() << "Unsupported obfuscation method"
                   << m_config.m_obfuscationMethod;
    return false;
  }
  m_process.setProgram(m_binaryFile);
  m_process.setArguments(args);

  logger.debug() << "Starting obfuscator";
  m_process.start();
  if (!m_process.waitForStarted(OBFUSCATOR_PROC_TIMEOUT_MS)) {
    logger.error() << "Failed to start obfuscator process"
                   << m_process.errorString();
    return false;
  }

  // Block until the helper either announces its port or exits
  while (m_process.state() == QProcess::Running) {
    if (!m_process.waitForReadyRead(OBFUSCATOR_PROC_TIMEOUT_MS)) {
      logger.error() << "Obfuscator did not announce a listening port";
      m_process.kill();
      m_process.waitForFinished(OBFUSCATOR_PROC_TIMEOUT_MS);
      return false;
    }
    while (m_process.canReadLine()) {
      const QByteArray line = m_process.readLine().trimmed();
      logger.debug() << "obf:" << QString::fromUtf8(line);
      const quint16 port = parseListeningPort(line);
      if (port != 0) {
        m_localPort = port;
        return true;
      }
    }
  }

  logger.error() << "Obfuscator exited before announcing a port"
                 << m_process.exitCode();
  return false;
}

quint16 QProcessObfuscator::parseListeningPort(const QByteArray& line) const {
  static const QByteArray prefix = "listening on 127.0.0.1:";
  const int idx = line.indexOf(prefix);
  if (idx < 0) {
    return 0;
  }
  bool ok = false;
  const quint16 port = line.mid(idx + prefix.size()).toUShort(&ok);
  return ok ? port : 0;
}

QStringList QProcessObfuscator::buildArgs(const InterfaceConfig& config,
                                          quint16 listenPort) const {
  QStringList args;

  const QString server = !config.m_serverIpv4AddrIn.isEmpty()
                             ? config.m_serverIpv4AddrIn
                             : config.m_serverIpv6AddrIn;
  args << QStringLiteral("--server") << server;
  args << QStringLiteral("--port") << QString::number(config.m_serverPort);
  if (listenPort != 0) {
    args << QStringLiteral("--listen-port") << QString::number(listenPort);
  }
#if defined(MZ_LINUX) && !defined(MZ_FLATPAK)
  args << QStringLiteral("--fwmark") << QString::number(WG_FIREWALL_MARK);
#endif

  switch (config.m_obfuscationMethod) {
    case Server::ObfuscationMethod::UdpOverTcp:
      args << QStringLiteral("udp-over-tcp");
      break;
    case Server::ObfuscationMethod::LWO:
      args << QStringLiteral("lwo");
      args << QStringLiteral("--lwo-version")
           << QStringLiteral("v%1").arg(config.m_lwoVersion);
      args << QStringLiteral("--public-key") << config.m_publicKey;
      args << QStringLiteral("--server-public-key") << config.m_serverPublicKey;
      break;
    default:
      return {};
  }

  return args;
}

QString QProcessObfuscator::binaryName() const {
#if defined(MZ_WINDOWS)
  return QStringLiteral("mozillavpn-obfuscator.exe");
#elif defined(MZ_LINUX)
  return QStringLiteral("mozillavpn-obfuscator");
#else
  logger.error() << "Obfuscation is not supported on this platform";
  return QString();
#endif
}

void QProcessObfuscator::stop() {
  if (m_process.state() == QProcess::NotRunning) {
    return;
  }
  logger.debug() << "Stopping obfuscator";
  m_process.terminate();
  if (!m_process.waitForFinished(OBFUSCATOR_PROC_TIMEOUT_MS)) {
    m_process.kill();
    m_process.waitForFinished(OBFUSCATOR_PROC_TIMEOUT_MS);
  }
}

QProcessObfuscator::~QProcessObfuscator() {
  MZ_COUNT_DTOR(QProcessObfuscator);
  stop();
}
