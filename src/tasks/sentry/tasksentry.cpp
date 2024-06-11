/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tasksentry.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>

#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "sentry/sentryadapter.h"
#include "settingsholder.h"

namespace {
Logger logger("TaskSentry");
}

TaskSentry::TaskSentry(const QByteArray& envelope) : Task("TaskSentry") {
  MZ_COUNT_CTOR(TaskSentry);
  m_envelope = envelope;

  connect(this, &TaskSentry::completed, [this]() { deleteLater(); });
}

TaskSentry::~TaskSentry() { MZ_COUNT_DTOR(TaskSentry); }

void TaskSentry::run() {
  // If it's unknown, try to parse it.
  if (m_Type == ContentType::Unknown) {
    m_Type = parseEnvelope();
  }
  // If it's still unknown, parsing failed. abort.
  if (m_Type == ContentType::Unknown) {
    logger.info() << "Dropping Sentry-Ping:Unknown";
    logger.info() << m_envelope;
    emit completed();
    return;
  }

  if (m_Type == ContentType::Ping) {
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
  Q_ASSERT(m_Type == ContentType::CrashReport);

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
  auto settings = SettingsHolder::instance();
  auto endpoint = settings->sentryEndpoint();
  auto dsn = settings->sentryDSN();
  if (endpoint.isNull() || dsn.isNull()) {
    // We should not be able to initialize Sentry without an endpoint and DSN
    Q_ASSERT(false);
    emit completed();
    return;
  }
  auto* request = new NetworkRequest(this, 200);
  QUrl target_endpoint(endpoint, QUrl::StrictMode);
  if (!target_endpoint.isValid()) {
    logger.error() << "Invalid URL for Sentry provided: " << endpoint;
    return;
  }
  request->requestInternal().setHeader(QNetworkRequest::ContentTypeHeader,
                                       "application/x-sentry-envelope");
  request->requestInternal().setRawHeader("dsn", dsn.toLocal8Bit());
  request->post(target_endpoint, m_envelope);

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
            if (!m_eventID.isEmpty()) {
              QString sentry_event_url(
                  "https://mozilla.sentry.io/discover/vpn-client:%0");
              logger.debug() << "Sentry reported crash: << "
                             << sentry_event_url.arg(m_eventID);
            }

            emit completed();
          });
}

TaskSentry::ContentType TaskSentry::parseEnvelope() {
  auto envelope = QString::fromUtf8(m_envelope);

  // The Sentry Envelope is a basic format
  // It is a series of JSON objects splited by lines.
  QStringList objects = envelope.split("\n");
  if (objects.empty()) {
    // This really should not happen.
    return ContentType::Unknown;
  }
  // The fist line is always the header:
  // a json object, containing the DSN and an event ID
  auto header = QJsonDocument::fromJson(objects.at(0).toUtf8());
  if (!header.isObject()) {
    return ContentType::Unknown;
  }

  // Each Line here is a json object.
  // Usually in the form of
  // { headerObject type } \n
  // { {bodyObject} || raw bytes} \n

  foreach (auto content, objects) {
    auto doc = QJsonDocument::fromJson(content.toUtf8());
    if (!doc.isObject()) {
      // This might be an attachment body.
      // Sniff for minidump bytes, otherwise this is
      // fine :)
      if (content.startsWith("MDMP")) {
        return ContentType::CrashReport;
      }
      // This is an attachment, fine to skip
      continue;
    }
    // The only thing right now, we don't want to send
    // crash-data, so we will only check for that.
    QJsonObject obj = doc.object();
    if (obj.contains("event_id")) {
      m_eventID = obj["event_id"].toString();
    }
    if (isCrashReportHeader(obj)) {
      return ContentType::CrashReport;
    }
  }
  return ContentType::Ping;
}

// static
bool TaskSentry::isCrashReportHeader(const QJsonObject& obj) {
  // Check 1: Test if this might be a dump attachment
  if (obj.contains("type")) {
    auto type = obj["type"].toString();
    // It's type minidump:
    if (type == "minidump") {
      return true;
    }
    if (type == "attachment") {
      // It's an attachment to an event.
      // If it's not a generic one
      // it needs to set 'attachment_type'
      if (!obj.contains("attachment_type")) {
        // This is a generic one
        return false;
      }
      auto type = obj["attachment_type"].toString();
      if (type == "event.minidump" || type == "event.applecrashreport") {
        // The attachment is a dump.
        return true;
      }
    }
    return false;
  }
  // Check 2: Check if this is a report for a crash
  if (obj.contains("level")) {
    // We might not send a minidump but still report a crash here :)
    auto level = obj["level"].toString();
    return level == "fatal";
  }
  return false;
}
