/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tasksentry.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>

#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"
#include "sentry/sentryadapter.h"
#include "sentry/sentrysniffer.h"
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
  if (m_Type == SentrySniffer::ContentType::Unknown) {
    auto const res = SentrySniffer::parseEnvelope(m_envelope);
    m_Type = res.type;
    m_eventID = res.id.value_or(QString());
  }
  // If it's still unknown, parsing failed. abort.
  if (m_Type == SentrySniffer::ContentType::Unknown) {
    logger.info() << "Dropping Sentry-Ping:Unknown";
    logger.info() << m_envelope;
    emit completed();
    return;
  }

  if (m_Type == SentrySniffer::ContentType::Ping) {
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
  Q_ASSERT(m_Type == SentrySniffer::ContentType::CrashReport);

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
  NetworkRequest* request = new NetworkRequest(this, 200);
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
