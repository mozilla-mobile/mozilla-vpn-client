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
    // The consent screen was now triggered;
    // exit here and connect to the change event to re-run.
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
            // Let's note the event id in the logs, so we can
            // connect customer support logs with sentry :)
            logger.debug() << "Sentry sent event (" << m_eventID << ")";
            emit completed();
          });
}

bool TaskSentry::isCrashReport() {
  if (m_checkedContent) {
    // We already checked this envelope, no need to do that twice.
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
  // The fist line is always the header:
  // a json object, containing the DSN and an event ID
  auto header = objects.takeFirst();
  if (!header.contains("\"event_id\":")) {
    // if there is no event-id, there will be no event following that
    // therefore this can't be a crashreport.
    // So we can stop parsing this blob. c:
    return;
  } else {
    // We have an event, let's keep the ID for logging.
    auto doc = QJsonDocument::fromJson(header.toUtf8());
    m_eventID = doc["event_id"].toString();
  }
  // Each Line here is it's own json object.
  // Usually in the form of
  // {{ headerObject type,len }} \n
  // {{ bodyObject meta}}
  foreach (auto content, objects) {
    auto doc = QJsonDocument::fromJson(content.toUtf8());
    if (!doc.isObject()) {
      logger.error() << "Invalid content read from the JSON file";
      continue;
    }
    // The only thing right now, we don't want to send
    // crash-data, so we will only check for that.
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
