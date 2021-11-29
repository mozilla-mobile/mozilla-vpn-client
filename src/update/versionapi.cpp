/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "versionapi.h"
#include "constants.h"
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
  logger.debug() << "VersionApi created";
}

VersionApi::~VersionApi() {
  MVPN_COUNT_DTOR(VersionApi);
  logger.debug() << "VersionApi released";
}

void VersionApi::start(Task* task) {
  NetworkRequest* request = NetworkRequest::createForVersions(task);

  connect(request, &NetworkRequest::requestFailed,
          [](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Request failed" << error;
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Request completed";

            if (!processData(data)) {
              logger.debug() << "Ignore failure.";
            }
          });

  connect(request, &QObject::destroyed, this, &QObject::deleteLater);
}

bool VersionApi::processData(const QByteArray& data) {
  QJsonDocument json = QJsonDocument::fromJson(data);
  if (!json.isObject()) {
    logger.error() << "A valid JSON object expected";
    return false;
  }

  QJsonObject obj = json.object();

  QString platformKey = Constants::PLATFORM_NAME;

  if (!obj.contains(platformKey)) {
    logger.debug() << "No key" << platformKey;
    return false;
  }

  QJsonValue platformDataValue = obj.value(platformKey);
  if (!platformDataValue.isObject()) {
    logger.error() << "Platform object not available";
    return false;
  }

  QJsonObject platformData = platformDataValue.toObject();

  QString latestVersion;
  QString minimumVersion;
  QString currentVersion(appVersion());

  QJsonValue latestValue = platformData.value("latest");
  if (!latestValue.isObject()) {
    logger.warning() << "Platform.latest object not available";
  } else {
    QJsonObject latestData = latestValue.toObject();

    QJsonValue latestVersionValue = latestData.value("version");
    if (!latestVersionValue.isString()) {
      logger.warning() << "Platform.latest.version string not available";
    } else {
      latestVersion = latestVersionValue.toString();
    }
  }

  QJsonValue minimumValue = platformData.value("minimum");
  if (!minimumValue.isObject()) {
    logger.warning() << "Platform.minimum object not available";
  } else {
    QJsonObject minimumData = minimumValue.toObject();

    QJsonValue minimumVersionValue = minimumData.value("version");
    if (!minimumVersionValue.isString()) {
      logger.warning() << "Platform.minimum.version string not available";
    } else {
      minimumVersion = minimumVersionValue.toString();
    }
  }

  logger.debug() << "Latest version:" << latestVersion;
  logger.debug() << "Minimum version:" << minimumVersion;
  logger.debug() << "Current version:" << currentVersion;

  if (compareVersions(currentVersion, minimumVersion) == -1) {
    logger.debug() << "update required";
    emit updateRequired();
    return true;
  }

  if (compareVersions(currentVersion, latestVersion) == -1) {
    logger.debug() << "Update recommended.";
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

// static
QString VersionApi::stripMinor(const QString& a) {
  QStringList aParts;

  if (!a.isEmpty()) {
    for (const QString& part : a.split(".")) aParts.append(part);
  }

  while (aParts.length() < 3) aParts.append("0");

  while (aParts.length() > 2) aParts.removeLast();

  aParts.append("0");
  return aParts.join(".");
}
