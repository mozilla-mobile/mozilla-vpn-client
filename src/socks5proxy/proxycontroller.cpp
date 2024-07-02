
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
constexpr auto const LOOPHOLE_BIN = "loophole.exe";
#else
constexpr auto const LOOPHOLE_BIN = "loophole";
#endif

namespace {
Logger logger("ProxyController");
}

const QString ProxyController::binaryPath() {
  auto binaryPath = QCoreApplication::applicationFilePath();
  auto info = QFileInfo(binaryPath);
  QDir dir = info.absoluteDir();
  return dir.filePath(LOOPHOLE_BIN);
}

void ProxyController::start() {
  auto loopholeFile = QFileInfo(binaryPath());
  if (!loopholeFile.exists()) {
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
        logger.error() << "Loophole Closed Unexpected with " << exitCode;
        logger.debug() << "Loophole scheudled restart";
        mCurrentProcess->deleteLater();
        start();
      });
  QObject::connect(mCurrentProcess, &QProcess::readyReadStandardOutput,
                   [this]() {
                     logger.debug() << "LOOPHOLE.exe"
                                    << mCurrentProcess->readAllStandardOutput();
                   });
  QObject::connect(mCurrentProcess, &QProcess::started, [this, url]() {
    logger.debug() << "Loophole available under:" << url.toString();
    m_state.setValue(Started{url});
  });
  QObject::connect(mCurrentProcess, &QProcess::errorOccurred,
                   [this](QProcess::ProcessError error) {
                     switch (error) {
                       case QProcess::ProcessError::Crashed:
                         logger.error() << "Loophole crashed!";
                         break;
                       case QProcess::ProcessError::FailedToStart:
                         logger.error() << "Loophole FailedToStart!";
                         break;
                       case QProcess::ProcessError::ReadError:
                         logger.error() << "Loophole ReadError!";
                         break;
                       case QProcess::ProcessError::Timedout:
                         logger.error() << "Loophole Timedout!";
                         break;
                       case QProcess::ProcessError::UnknownError:
                         logger.error() << "Loophole UnknownError!";
                         break;
                       case QProcess::ProcessError::WriteError:
                         logger.error() << "Loophole UnknownError!";
                         break;
                     }
                     stop();
                   });
  mCurrentProcess->setProgram(loopholeFile.absoluteFilePath());
  mCurrentProcess->setArguments(getArguments(url));
  logger.error() << "Try to start Loophole!  "
                 << loopholeFile.absoluteFilePath() << getArguments(url);
  mCurrentProcess->start(QIODeviceBase::ReadOnly);
}

void ProxyController::stop() {
  if (!mCurrentProcess && std::holds_alternative<Stopped>(m_state.value())) {
    return;
  }
  logger.debug() << "Loophole stopping";
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
