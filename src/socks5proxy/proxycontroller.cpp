
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
  if (!info.exists()) {
    Q_ASSERT(false);
  }
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
  mCurrentProcess->setProgram(loopholeFile.absoluteFilePath());
  mCurrentProcess->setArguments(getArguments(url));
  mCurrentProcess->start(QIODeviceBase::ReadOnly);
  mCrashSignal = QObject::connect(
      mCurrentProcess, &QProcess::finished,
      [this, url](int exitCode, QProcess::ExitStatus) {
        logger.error() << "Loophole Closed Unexpected with " << exitCode;
        logger.debug() << "Loophole scheudled restart";
        mCurrentProcess->deleteLater();
        start();
      });
  logger.debug() << "Loophole available under:" << url.toString();
  m_state.setValue(Started{url});
}

void ProxyController::stop() {
  logger.debug() << "Loophole stopping";
  if (!mCurrentProcess) {
    m_state = Stopped{};
    return;
  }
  // We no longer need signals from this obj
  QObject::disconnect(mCrashSignal);
  m_state = Stopped{};
  mCurrentProcess->kill();
  mCurrentProcess->deleteLater();
}

bool ProxyController::canActivate() {
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
  out.setScheme("socks5");
  out.setHost("localhost");
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
