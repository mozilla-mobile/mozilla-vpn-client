/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "releasemonitor.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "timersingleshot.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace {
Logger logger(LOG_MAIN, "ReleaseMonitor");
}

ReleaseMonitor::ReleaseMonitor() {
  MVPN_COUNT_CTOR(ReleaseMonitor);

  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, this, &ReleaseMonitor::runInternal);
}

ReleaseMonitor::~ReleaseMonitor() { MVPN_COUNT_DTOR(ReleaseMonitor); }

void ReleaseMonitor::runSoon() {
  logger.log() << "ReleaseManager - Scheduling a quick timer";
  TimerSingleShot::create(this, 0, [this] { runInternal(); });
}

void ReleaseMonitor::runInternal() {
  logger.log() << "ReleaseMonitor started";

  NetworkRequest* request = NetworkRequest::createForVersions(this);

  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.log() << "Versions request failed" << error;
            emit releaseChecked();
            schedule();
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this](const QByteArray& data) {
            logger.log() << "Account request completed";

            if (!processData(data)) {
              logger.log() << "Ignore failure.";
            }

            emit releaseChecked();
            schedule();
          });
}

void ReleaseMonitor::schedule() {
  logger.log() << "ReleaseMonitor scheduling";
  m_timer.start(Constants::RELEASE_MONITOR_MSEC);
}

bool ReleaseMonitor::processData(const QByteArray& data) {
  QJsonDocument json = QJsonDocument::fromJson(data);
  if (!json.isObject()) {
    logger.log() << "A valid JSON object expected";
    return false;
  }

  QJsonObject obj = json.object();

  QString platformKey =
#if defined(MVPN_IOS)
      "ios"
#elif defined(MVPN_MACOS)
      "macos"
#elif defined(MVPN_LINUX)
      "linux"
#elif defined(MVPN_ANDROID)
      "android"
#elif defined(MVPN_WINDOWS)
      "windows"
#elif defined(UNIT_TEST) || defined(MVPN_DUMMY)
      "dummy"
#else
#  error "Unsupported platform"
#endif
      ;

  if (!obj.contains(platformKey)) {
    logger.log() << "No key" << platformKey;
    return false;
  }

  QJsonValue platformDataValue = obj.value(platformKey);
  if (!platformDataValue.isObject()) {
    logger.log() << "Platform object not available";
    return false;
  }

  QJsonObject platformData = platformDataValue.toObject();

  QString latestVersion;
  QString minimumVersion;
  QString currentVersion(APP_VERSION);

  QJsonValue latestValue = platformData.value("latest");
  if (!latestValue.isObject()) {
    logger.log() << "Platform.latest object not available";
  } else {
    QJsonObject latestData = latestValue.toObject();

    QJsonValue latestVersionValue = latestData.value("version");
    if (!latestVersionValue.isString()) {
      logger.log() << "Platform.latest.version string not available";
    } else {
      latestVersion = latestVersionValue.toString();
    }
  }

  QJsonValue minimumValue = platformData.value("minimum");
  if (!minimumValue.isObject()) {
    logger.log() << "Platform.minimum object not available";
  } else {
    QJsonObject minimumData = minimumValue.toObject();

    QJsonValue minimumVersionValue = minimumData.value("version");
    if (!minimumVersionValue.isString()) {
      logger.log() << "Platform.minimum.version string not available";
    } else {
      minimumVersion = minimumVersionValue.toString();
    }
  }

  logger.log() << "Latest version:" << latestVersion;
  logger.log() << "Minimum version:" << minimumVersion;
  logger.log() << "Current version:" << currentVersion;

  if (compareVersions(currentVersion, minimumVersion) == -1) {
    logger.log() << "ReleaseMonitor - update required";
    MozillaVPN::instance()->setUpdateRecommended(false);
    MozillaVPN::instance()->controller()->updateRequired();
    return true;
  }

  bool recommended = compareVersions(currentVersion, latestVersion) == -1;
  logger.log() << "Update recommended: " << recommended;
  MozillaVPN::instance()->setUpdateRecommended(recommended);
  return true;
}

// static
int ReleaseMonitor::compareVersions(const QString& a, const QString& b) {
  if (a == b) {
    return 0;
  }

  if (a.isEmpty()) {
    return 1;
  }

  if (b.isEmpty()) {
    return -1;
  }

  QList<uint32_t> aParts;
  for (const QString& part : a.split(".")) aParts.append(part.toInt());

  while (aParts.length() < 3) aParts.append(0);

  QList<uint32_t> bParts;
  for (const QString& part : b.split(".")) bParts.append(part.toInt());

  while (bParts.length() < 3) bParts.append(0);

  // Major version number.
  for (uint32_t i = 0; i < 3; ++i) {
    if (aParts[i] != bParts[i]) {
      return aParts[i] < bParts[i] ? -1 : 1;
    }
  }

  return 0;
}
