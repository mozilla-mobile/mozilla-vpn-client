/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "versionapi.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QVersionNumber>

#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

namespace {
Logger logger("VersionApi");
}

VersionApi::VersionApi(QObject* parent) : Updater(parent) {
  MZ_COUNT_CTOR(VersionApi);
  logger.debug() << "VersionApi created";
}

VersionApi::~VersionApi() {
  MZ_COUNT_DTOR(VersionApi);
  logger.debug() << "VersionApi released";
}

void VersionApi::start(Task* task) {
  NetworkRequest* request = new NetworkRequest(task, 200);
  request->get(Constants::apiUrl(Constants::Versions));

  connect(request, &NetworkRequest::requestFailed, request,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Request failed" << error;
            deleteLater();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Request completed";

            if (!processData(data)) {
              logger.debug() << "Ignore failure.";
            }
            deleteLater();
          });
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

  QVersionNumber latestVersion;
  QVersionNumber minVersion;
  QVersionNumber currentVersion = QVersionNumber::fromString(appVersion());

  QJsonValue latestValue = platformData.value("latest");
  if (!latestValue.isObject()) {
    logger.warning() << "Platform.latest object not available";
  } else {
    QJsonValue latestVersionValue = latestValue["version"];
    if (!latestVersionValue.isString()) {
      logger.warning() << "Platform.latest.version string not available";
    } else {
      latestVersion = QVersionNumber::fromString(latestVersionValue.toString());
    }
  }

  QJsonValue minValue = platformData.value("minimum");
  if (!minValue.isObject()) {
    logger.warning() << "Platform.minimum object not available";
  } else {
    QJsonValue minVersionValue = minValue["version"];
    if (!minVersionValue.isString()) {
      logger.warning() << "Platform.minimum.version string not available";
    } else {
      minVersion = QVersionNumber::fromString(minVersionValue.toString());
    }
  }

  logger.debug() << "Latest version:" << latestVersion.toString();
  logger.debug() << "Minimum version:" << minVersion.toString();
  logger.debug() << "Current version:" << currentVersion.toString();

  if (currentVersion < minVersion) {
    logger.debug() << "update required";
    emit updateRequired();
    return true;
  }

  if (currentVersion < latestVersion) {
    logger.debug() << "Update recommended.";
    emit updateRecommended();
  }
  return true;
}
