/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "versionapi.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace {
Logger logger(LOG_NETWORKING, "VersionApi");
}

VersionApi::VersionApi(QObject* parent) : Updater(parent) {
  MVPN_COUNT_CTOR(VersionApi);
  logger.log() << "VersionApi created";
}

VersionApi::~VersionApi() {
  MVPN_COUNT_DTOR(VersionApi);
  logger.log() << "VersionApi released";
}

void VersionApi::start() {
  NetworkRequest* request = NetworkRequest::createForVersions(this);

  connect(request, &NetworkRequest::requestFailed,
          [](QNetworkReply*, QNetworkReply::NetworkError error,
             const QByteArray&) { logger.log() << "Request failed" << error; });

  connect(request, &NetworkRequest::requestCompleted,
          [this](QNetworkReply*, const QByteArray& data) {
            logger.log() << "Request completed";

            if (!processData(data)) {
              logger.log() << "Ignore failure.";
            }
          });

  connect(request, &QObject::destroyed, this, &QObject::deleteLater);
}

bool VersionApi::processData(const QByteArray& data) {
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
    logger.log() << "update required";
    emit updateRequired();
    return true;
  }

  if (compareVersions(currentVersion, latestVersion) == -1) {
    logger.log() << "Update recommended.";
    emit updateRecommended();
  }
  return true;
}

// static
int VersionApi::compareVersions(const QString& a, const QString& b) {
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
