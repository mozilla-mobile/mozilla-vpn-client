
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "proxycontroller.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QRandomGenerator>

#include "constants.h"
#include "feature/feature.h"
#include "logger.h"

#ifdef MZ_WINDOWS
#  include "platforms/windows/windowscommons.h"
constexpr auto const SOCKSPROXY_BIN = "socksproxy.exe";
#else
constexpr auto const SOCKSPROXY_BIN = "socksproxy";
#endif

namespace {
Logger logger("ProxyController");
}

const QString ProxyController::binaryPath() {
  auto binaryPath = QCoreApplication::applicationFilePath();
  auto info = QFileInfo(binaryPath);
  QDir dir = info.absoluteDir();
  return dir.filePath(SOCKSPROXY_BIN);
}

void ProxyController::start() {
  auto socksProxyFile = QFileInfo(binaryPath());
  if (!socksProxyFile.exists()) {
    Q_ASSERT(false);
    return;
  }

  if (mCurrentProcess) {
    mCurrentProcess->kill();
    mCurrentProcess->deleteLater();
  }
  auto const url = generateProxyUrl();

  mCurrentProcess = new QProcess();
  mCrashSignal = QObject::connect(
      mCurrentProcess, &QProcess::finished,
      [this, url](int exitCode, QProcess::ExitStatus) {
        logger.error() << "SocksProxy Closed Unexpected with " << exitCode;
        logger.debug() << "SocksProxy scheudled restart";
        mCurrentProcess->deleteLater();
        start();
      });
  QObject::connect(mCurrentProcess, &QProcess::readyReadStandardOutput,
                   [this]() {
                     logger.debug() << "SocksProxy.exe"
                                    << mCurrentProcess->readAllStandardOutput();
                   });
  QObject::connect(mCurrentProcess, &QProcess::started, [this, url]() {
    logger.debug() << "SocksProxy available under:" << url.toString();
    m_state.setValue(Started{url});
  });
  QObject::connect(mCurrentProcess, &QProcess::errorOccurred,
                   [this](QProcess::ProcessError error) {
                     switch (error) {
                       case QProcess::ProcessError::Crashed:
                         logger.error() << "SocksProxy crashed!";
                         break;
                       case QProcess::ProcessError::FailedToStart:
                         logger.error() << "SocksProxy FailedToStart!";
                         break;
                       case QProcess::ProcessError::ReadError:
                         logger.error() << "SocksProxy ReadError!";
                         break;
                       case QProcess::ProcessError::Timedout:
                         logger.error() << "SocksProxy Timedout!";
                         break;
                       case QProcess::ProcessError::UnknownError:
                         logger.error() << "SocksProxy UnknownError!";
                         break;
                       case QProcess::ProcessError::WriteError:
                         logger.error() << "SocksProxy UnknownError!";
                         break;
                     }
                     stop();
                   });
  mCurrentProcess->setProgram(socksProxyFile.absoluteFilePath());
  mCurrentProcess->setArguments(getArguments(url));
  logger.error() << "Try to start SocksProxy!  "
                 << socksProxyFile.absoluteFilePath() << getArguments(url);
  mCurrentProcess->start(QIODeviceBase::ReadOnly);
}

void ProxyController::stop() {
  if (!mCurrentProcess && std::holds_alternative<Stopped>(m_state.value())) {
    return;
  }
  logger.debug() << "SocksProxy stopping";
  // We no longer need signals from this obj
  m_state = Stopped{};
  mCurrentProcess->kill();
}

bool ProxyController::canActivate() {
  // This is not for prod rn.
  if (Constants::inProduction()) {
    return false;
  }
#ifndef MZ_WINDOWS
  return false;
#else
  auto const* f = Feature::get(Feature::Feature_splitTunnel);
  return f->isSupported() && QFileInfo(binaryPath()).exists();
#endif
}

// static
QUrl ProxyController::generateProxyUrl() {
  QUrl out = {};
  out.setScheme("socks");
  out.setHost("127.0.0.1");
  out.setPort(
      static_cast<uint16_t>(QRandomGenerator::global()->bounded(49152, 65535)));

  // TODO: Add username/pass combo
  return out;
}

// static
const QStringList ProxyController::getArguments(const QUrl& proxyURL) {
  QStringList out{};

  out.append("-p " + QString::number(proxyURL.port()));
  out.append("-a " + proxyURL.host());

  if (!proxyURL.userInfo().isEmpty()) {
    out.append("-P " + proxyURL.password());
    out.append("-U " + proxyURL.userName());
  }
  return out;
}
