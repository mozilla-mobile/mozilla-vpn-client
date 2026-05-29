/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "obfuscator.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

#include "leakdetector.h"
#include "logger.h"

constexpr int OBFUSCATOR_PROC_TIMEOUT_MS = 5000;
#ifdef MZ_LINUX
constexpr uint32_t WG_FIREWALL_MARK = 0xca6c;
#endif

namespace {
Logger logger("Obfuscator");
}

Obfuscator::Obfuscator(const InterfaceConfig& config) {
  MZ_COUNT_CTOR(Obfuscator);

  const QStringList args = buildArgs(config);
  if (args.isEmpty()) {
    logger.error() << "Unsupported obfuscation method"
                   << config.m_obfuscationMethod;
    return;
  }

  const QString binary = binaryName();
  m_process.setProgram(binary);
  m_process.setArguments(args);
  // Merge stderr into stdout so we can read the "listening on" announce line
  m_process.setProcessChannelMode(QProcess::MergedChannels);
}

bool Obfuscator::start() {
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

quint16 Obfuscator::parseListeningPort(const QByteArray& line) const {
  static const QByteArray prefix = "listening on 127.0.0.1:";
  const int idx = line.indexOf(prefix);
  if (idx < 0) {
    return 0;
  }
  bool ok = false;
  const quint16 port = line.mid(idx + prefix.size()).toUShort(&ok);
  return ok ? port : 0;
}

QStringList Obfuscator::buildArgs(const InterfaceConfig& config) {
  QStringList args;
  switch (config.m_obfuscationMethod) {
    case Server::ObfuscationMethod::UdpOverTcp:
      args << QStringLiteral("udp-over-tcp");
      break;
    default:
      return {};
  }

  const QString server = !config.m_serverIpv4AddrIn.isEmpty()
                             ? config.m_serverIpv4AddrIn
                             : config.m_serverIpv6AddrIn;
  args << QStringLiteral("--server") << server;
  args << QStringLiteral("--port") << QString::number(config.m_serverPort);
#ifdef MZ_LINUX
  args << QStringLiteral("--fwmark") << QString::number(WG_FIREWALL_MARK);
#endif
  // The obfuscator needs to bind to the same interface as the WireGuard peer
  return args;
}

QString Obfuscator::binaryName() const {
#if defined(MZ_WINDOWS)
  return QStringLiteral("mozillavpn-obfuscator.exe");
#elif defined(MZ_LINUX)
  return QStringLiteral("mozillavpn-obfuscator");
#else
  logger.error() << "Obfuscation is not supported on this platform";
  return QString();
#endif
}

Obfuscator::~Obfuscator() {
  MZ_COUNT_DTOR(Obfuscator);
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
