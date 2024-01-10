/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tasksentryconfig.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>

#include "context/constants.h"
#include "leakdetector.h"
#include "logging/logger.h"
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

  QUrl url(Constants::apiBaseUrl());
  url.setPath("/api/v1/vpn/crashreporting");

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError, const QByteArray&) {
            logger.error() << "Failed to get Sentry info";
            emit completed();
          });
  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            parseSentryConfig(data);
            emit completed();
          });

  request->get(url);
}

void TaskSentryConfig::parseSentryConfig(const QByteArray& data) {
  auto doc = QJsonDocument::fromJson(data);
  if (!doc.isObject()) {
    logger.error() << "Failed to get Sentry info";
    return;
  }
  QJsonObject obj = doc.object();
  if (!obj.contains("dsn")) {
    logger.error() << "DSN missing from Sentry info";
    return;
  };
  auto dsn = obj["dsn"].toString();
  if (dsn.isNull()) {
    logger.error() << "DSN missing from Sentry info";
    return;
  };

  if (!obj.contains("endpoint")) {
    logger.error() << "endpoint missing from Sentry info";
    return;
  };
  auto endpoint = obj["endpoint"].toString();
  if (endpoint.isNull()) {
    logger.error() << "Sentry provided endpoint is null";
    return;
  };
  if (!QUrl(endpoint).isValid()) {
    logger.error() << "Sentry provided endpoint is not a valid url";
    return;
  }
  auto settings = SettingsHolder::instance();
  settings->setSentryEndpoint(endpoint);
  settings->setSentryDSN(dsn);

  SentryAdapter::instance()->init();
}
