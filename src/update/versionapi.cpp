/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "versionapi.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "constants.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "versionutils.h"

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

  if (VersionUtils::compareVersions(currentVersion, minimumVersion) == -1) {
    logger.debug() << "update required";
    emit updateRequired();
    return true;
  }

  if (VersionUtils::compareVersions(currentVersion, latestVersion) == -1) {
    logger.debug() << "Update recommended.";
    emit updateRecommended();
  }
  return true;
}
