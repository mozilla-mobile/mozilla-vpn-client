
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "proxycontroller.h"

#include <QObject.h>

#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QProcess>
#include <QRandomGenerator>
#include <QTimer>

#include "feature/feature.h"
#include "logger.h"

#ifdef MZ_WINDOWS
constexpr auto const SOCKSPROXY_BIN = "socksproxy.exe";
#else
constexpr auto const SOCKSPROXY_BIN = "socksproxy";
#endif

namespace {
Logger logger("ProxyController");
}

const QString ProxyController::binaryPath() {
// On Release expect it to be in the same folder
// as the executable
#ifndef MZ_DEBUG
  auto binaryPath = QCoreApplication::applicationFilePath();
  auto info = QFileInfo(binaryPath);
  QDir dir = info.absoluteDir();
  return dir.filePath(SOCKSPROXY_BIN);
#else
  auto binaryPath = QCoreApplication::applicationFilePath();
  auto currentDir = QFileInfo(binaryPath).dir();
  auto socksproxydir = QFileInfo(currentDir, "../extension/socks5proxy/bin/");
  return socksproxydir.absoluteDir().filePath(SOCKSPROXY_BIN);
#endif
}

void ProxyController::start() {
  if (mCurrentProcess && std::holds_alternative<Started>(m_state.value())) {
    return;
  }
  auto socksProxyFile = QFileInfo(binaryPath());
  if (!socksProxyFile.exists()) {
    Q_ASSERT(false);
    return;
  }

  if (mCurrentProcess) {
    // It should not be alive q_q
    mCurrentProcess->kill();
    mCurrentProcess->waitForFinished();
    mCurrentProcess->deleteLater();
  }
  auto const url = generateProxyUrl();

  mCurrentProcess = new QProcess();
  mCrashSignal = QObject::connect(
      mCurrentProcess, &QProcess::finished,
      [this](int exitCode, QProcess::ExitStatus) {
        logger.error() << "SocksProxy Closed Unexpected with " << exitCode;
        mCurrentProcess->deleteLater();
        using namespace std::literals::chrono_literals;
        // scheudle a restart
        QTimer::singleShot(100ms, [this]() { start(); });
        logger.debug() << "SocksProxy scheudled restart";
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
  mErrorSignal =
      QObject::connect(mCurrentProcess, &QProcess::errorOccurred,
                       [](QProcess::ProcessError error) {
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
                       });
  mCurrentProcess->setProgram(socksProxyFile.absoluteFilePath());
  mCurrentProcess->setArguments(getArguments(url));
  logger.error() << "Try to start SocksProxy!  "
                 << socksProxyFile.absoluteFilePath()
                 << getArguments(url).join("");
  mCurrentProcess->start(QIODeviceBase::ReadOnly);
}

void ProxyController::stop() {
  if (!mCurrentProcess && std::holds_alternative<Stopped>(m_state.value())) {
    // We already stopped.
    return;
  }
  // We're stopping the process, so disconnect
  // the listner to restart the process.
  QObject::disconnect(mCrashSignal);
  QObject::disconnect(mErrorSignal);
  logger.debug() << "SocksProxy stopping";
  // We no longer need signals from this obj
  m_state = Stopped{};
  // Once the Process is stopped, we can delete it
  QObject::connect(mCurrentProcess, &QProcess::finished,
                   [p = mCurrentProcess.get()]() {
                     logger.info() << "Deleting proxy with args "
                                   << p->arguments() << " has stopped.";
                     p->deleteLater();
                   });
  mCurrentProcess->kill();
}

bool ProxyController::canActivate() {
  if (!m_canActivate.has_value()) {
    m_canActivate = ([]() {
      auto const* f = Feature::get(Feature::Feature_localProxy);
      if (!f->isSupported()) {
        logger.info() << "SocksProxy disabled due to Feature Flag";
        return false;
      }
      if (!QFileInfo(binaryPath()).exists()) {
        logger.info() << "SocksProxy disabled due to Missing Binary";
        logger.info() << "Expected socksproxy here: " << binaryPath();
        return false;
      }
      logger.info() << "Socksproxy is supported";
      return true;
    })();
  }
  return m_canActivate.value();
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
