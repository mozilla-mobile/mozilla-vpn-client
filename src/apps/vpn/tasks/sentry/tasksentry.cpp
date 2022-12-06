/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tasksentry.h"

#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "sentry/sentryadapter.h"
#include "settingsholder.h"

#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace {
Logger logger("TaskSentry");
}

TaskSentry::TaskSentry(const QByteArray& envelope) : Task("TaskSentry") {
  MZ_COUNT_CTOR(TaskSentry);
  m_envelope = envelope;
}

TaskSentry::~TaskSentry() { MZ_COUNT_DTOR(TaskSentry); }

void TaskSentry::run() {
  if (!isCrashReport()) {
    // This is not a crash report, but a ping
    if (SettingsHolder::instance()->gleanEnabled()) {
      // In case telemetry is enabled, send the request
      logger.info() << "Sending Sentry-Ping";
      sendRequest();
    } else {
      logger.info() << "Dropping Sentry-Ping: No Consent";
      // Throw it away.
      emit completed();
    }
    return;
  }

  // This is a crash report - Make sure we have consent, or ask the user for it
  auto consent = SentryAdapter::instance()->hasCrashUploadConsent();

  if (consent == SentryAdapter::UserConsentResult::Pending) {
    // We as it's unknown - The consent screen was now triggered
    // exit here and connect to re-run once that changed.
    connect(SentryAdapter::instance(), &SentryAdapter::userConsentChanged, this,
            &TaskSentry::run);
    logger.info() << "Halting Sentry-Report: No Consent";
    return;
  }
  if (consent == SentryAdapter::UserConsentResult::Forbidden) {
    // We are asked not to send anything, let's drop it all.
    logger.info() << "Dropping Sentry-Report: No Consent";
    emit completed();
    return;
  }
  logger.info() << "Sending Sentry-Report: Got Consent";
  sendRequest();
}

void TaskSentry::sendRequest() {
  NetworkRequest* request = NetworkRequest::createForSentry(this, m_envelope);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            Q_UNUSED(error);
            logger.error() << "Failed to send envelope";
            emit completed();
          });
  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            Q_UNUSED(data);
            logger.debug() << "Sentry sent event (" << m_eventID << ")";
            emit completed();
          });
}

bool TaskSentry::isCrashReport() {
  if (m_checkedContent) {
    // We already checked this envelope, no need 2 do that twice.
    return m_isCrashReport;
  }
  parseEnvelope();
  m_checkedContent = true;
  return m_isCrashReport;
}

void TaskSentry::parseEnvelope() {
  auto envelope = QString::fromUtf8(m_envelope);

  // The Sentry Envelope is a basic format
  // It is a series of JSON objects splited by lines.
  QStringList objects = envelope.split("\n");
  if (objects.empty()) {
    Q_UNREACHABLE();  // This really should not happen.
  }
  // If the header does not already have an event-id
  // there is not going to be an event in the body.
  // We can shortcut here, as nothing else interests us rn.
  auto header = objects.takeFirst();
  if (!header.contains("\"event_id\":")) {
    return;
  } else {
    auto doc = QJsonDocument::fromJson(header.toUtf8());
    m_eventID = doc["event_id"].toString();
  }
  // Each of those is a json string
  foreach (auto content, objects) {
    auto doc = QJsonDocument::fromJson(content.toUtf8());
    if (!doc.isObject()) {
      logger.error() << "Invalid content read from the JSON file";
      continue;
    }
    QJsonObject obj = doc.object();
    QJsonValue metadata = obj["debug_meta"];
    if (metadata.isUndefined() || metadata.isNull()) {
      continue;  // Does not contain crash metadata
    }
    if (!metadata.isObject()) {
      continue;  // Probably malformed?
    }
    if (metadata.toObject().contains("images")) {
      // 100% a crashreport
      m_isCrashReport = true;
    }
  }
}
