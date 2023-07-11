/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tasksentryconfig.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>

#include "appconstants.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "sentry/sentryadapter.h"
#include "settingsholder.h"

namespace {
Logger logger("TaskSentryConfig");
}

TaskSentryConfig::TaskSentryConfig() : Task("TaskSentryConfig") {
  MZ_COUNT_CTOR(TaskSentryConfig);
}

TaskSentryConfig::~TaskSentryConfig() { MZ_COUNT_DTOR(TaskSentryConfig); }

void TaskSentryConfig::run() {
  NetworkRequest* request = new NetworkRequest(this, 200);

  QUrl url(AppConstants::apiBaseUrl());
  url.setPath("/api/v1/vpn/crashreporting");

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            Q_UNUSED(error);
            logger.error() << "Failed to get Sentry info";
            emit completed();
          });
  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            if (handleNetworkResponse(data)) {
              // We now should have sentry info - let's init
              SentryAdapter::instance()->init();
            }
            emit completed();
          });

  request->get(url);
}

bool TaskSentryConfig::handleNetworkResponse(const QByteArray& data) {
  auto doc = QJsonDocument::fromJson(data);
  if (!doc.isObject()) {
    logger.error() << "Failed to get Sentry info";
    return false;
  }
  QJsonObject obj = doc.object();
  if (!obj.contains("dsn")) {
    logger.error() << "DSN missing from Sentry info";
    return false;
  };
  auto dsn = obj["dsn"].toString();
  if (dsn.isNull()) {
    logger.error() << "DSN missing from Sentry info";
    return false;
  };

  if (!obj.contains("endpoint")) {
    logger.error() << "endpoint missing from Sentry info";
    return false;
  };
  auto endpoint = obj["endpoint"].toString();
  if (endpoint.isNull()) {
    logger.error() << "Sentry provided endpoint is null";
    return false;
  };
  auto settings = SettingsHolder::instance();
  settings->setSentryEndpoint(endpoint);
  settings->setSentryDSN(dsn);
  return true;
}