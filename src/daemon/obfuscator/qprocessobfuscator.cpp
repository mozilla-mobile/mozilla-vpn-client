/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "qprocessobfuscator.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QTimer>

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

QProcessObfuscator::QProcessObfuscator(
    const QString& path, const InterfaceConfig& config, QObject* parent)
    : Obfuscator(parent), m_config(config) {
  MZ_COUNT_CTOR(QProcessObfuscator);

  m_process.setProgram(path);
  m_process.setProcessChannelMode(QProcess::SeparateChannels);
  connect(&m_process, &QProcess::readyReadStandardError, this,
          &QProcessObfuscator::stderrReady);
  connect(&m_process, &QProcess::readyReadStandardOutput, this,
          &QProcessObfuscator::stdoutReady);
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
  const QStringList args = buildArgs(m_config, listenPort);
  if (args.isEmpty()) {
    logger.error() << "Unsupported obfuscation method"
                   << m_config.m_obfuscationMethod;
    return false;
  }
  m_process.setArguments(args);

  QTimer deadline;
  deadline.setSingleShot(true);
  deadline.start(OBFUSCATOR_PROC_TIMEOUT_MS);

  QEventLoop loop;
  connect(this, &QProcessObfuscator::started, &loop, &QEventLoop::exit);
  connect(&deadline, &QTimer::timeout, &loop, [&loop]() { loop.exit(); });
  connect(&m_process, &QProcess::errorOccurred, &loop,
          [&](QProcess::ProcessError) {
            logger.error() << "Failed to start obfuscator process"
                           << m_process.errorString();
            loop.exit();
          });
  connect(&m_process, &QProcess::finished, &loop,
          [&loop](int code, QProcess::ExitStatus status) {
            if (status == QProcess::CrashExit) {
              logger.error() << "Obfuscator crashed with signal" << code;
            } else {
              logger.error() << "Obfuscator exited before announcing a port";
            }
            loop.exit();
          });

  logger.debug() << "Starting obfuscator";
  m_process.start();
  m_localPort = loop.exec();
  return (m_localPort != 0);
}

void QProcessObfuscator::stderrReady() {
  m_process.setReadChannel(QProcess::StandardError);
  for (;;) {
    QString line = QString::fromUtf8(m_process.readLine());
    if (line.isEmpty()) {
      break;
    }
    logger.warning() << line;
  }
}

void QProcessObfuscator::stdoutReady() {
  static const QString prefix("listening on 127.0.0.1:");

  m_process.setReadChannel(QProcess::StandardOutput);
  for (;;) {
    QString line = QString::fromUtf8(m_process.readLine());
    if (line.isEmpty()) {
      break;
    }
    if (line.startsWith(prefix)) {
      emit started(line.sliced(prefix.length()).toUShort());
    }
    logger.info() << line;
  }
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

void QProcessObfuscator::stop() {
  if (m_process.state() == QProcess::NotRunning) {
    return;
  }
  logger.debug() << "Stopping obfuscator";

  // On Windows terminate() posts WM_CLOSE, which a console app without a
  // message loop never receives so we just kill it.  
#if !defined(MZ_WINDOWS)
  m_process.terminate();
  if (m_process.waitForFinished(OBFUSCATOR_PROC_TIMEOUT_MS)) {
    return;
  }
#endif

  // On Windows terminate() posts WM_CLOSE, which a console app without a
  // message loop never receives so we just kill it.
  m_process.kill();
  m_process.waitForFinished(OBFUSCATOR_PROC_TIMEOUT_MS);
}

QProcessObfuscator::~QProcessObfuscator() {
  MZ_COUNT_DTOR(QProcessObfuscator);
  stop();
}
