/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "iosupdater.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "constants.h"
#include "iosutils.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "versionutils.h"

namespace {
Logger logger("IOSUpdater");
}

IOSUpdater::IOSUpdater(QObject* parent) : Updater(parent) {
  MZ_COUNT_CTOR(IOSUpdater);
  logger.debug() << "IOSUpdater created";
}

IOSUpdater::~IOSUpdater() {
  MZ_COUNT_DTOR(IOSUpdater);
  logger.debug() << "IOSUpdater released";
}

void IOSUpdater::start(Task* task) {
  m_versionApi = new VersionApi(this);
  connect(m_versionApi, &Updater::updateRequired, this, [task, this] {
    m_updateRequired = true;
    checkVersion(task, m_versionApi->latestVersion());
  });

  connect(m_versionApi, &Updater::updateRecommended, this,
          [task, this] { checkVersion(task, m_versionApi->latestVersion()); });

  connect(m_versionApi, &QObject::destroyed, this, [this] {
    if (!m_versionApi->recommendedOrRequired()) {
      deleteLater();
    }
  });

  m_versionApi->start(task);
}

void IOSUpdater::checkVersion(Task* task, const QString& latestVersion) {
  QString url = QString("http://itunes.apple.com/lookup?bundleId=%1")
                    .arg(QString::fromNSString(IOSUtils::appId()));
  NetworkRequest* request = NetworkRequest::createForGetUrl(task, url, 200);

  connect(request, &NetworkRequest::requestFailed, request,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Request failed" << error;
            deleteLater();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this, latestVersion](const QByteArray& data) {
            logger.debug() << "Request completed";

            if (!processData(latestVersion, data)) {
              logger.debug() << "Ignore failure.";
            }
            deleteLater();
          });
}

bool IOSUpdater::processData(const QString& latestVersion, const QByteArray& data) {
  QJsonDocument json = QJsonDocument::fromJson(data);
  if (!json.isObject()) {
    return false;
  }

  QJsonArray releases = json.object()["results"].toArray();
  if (releases.isEmpty()) {
    return false;
  }

  QString currentVersion = appVersion();

  for (const QJsonValue& value : releases) {
    QString version = value.toObject()["version"].toString();
    // version is Apple-reported (from iTunes request above), latestVersion is Guardian-reported (from VersionApi request). Only continue if Apple is able to provide the app version Guardian expects it to.
    if (VersionUtils::compareVersions(version, latestVersion) != 0) {
      continue;
    }

    if (m_updateRequired) {
      emit updateRequired();
    } else {
      emit updateRecommended();
    }
  }

  return true;
}
