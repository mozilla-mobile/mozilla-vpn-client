/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "sentryadapter.h"

#include <sentry.h>

#include <QDir>
#include <QStandardPaths>

#include "constants.h"
#include "feature.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"
#include "settingsholder.h"
#include "tasks/sentry/tasksentry.h"
#include "taskscheduler.h"

namespace {
SentryAdapter* s_instance = nullptr;
Logger logger("Sentry");

}  // namespace

SentryAdapter* SentryAdapter::instance() {
  if (s_instance == nullptr) {
    s_instance = new SentryAdapter();
  }
  return s_instance;
}
SentryAdapter::SentryAdapter() { MZ_COUNT_CTOR(SentryAdapter); }
SentryAdapter::~SentryAdapter() { MZ_COUNT_DTOR(SentryAdapter); }

void SentryAdapter::init() {
  if (!Feature::get(Feature::Feature_sentry)->isSupported()) {
    return;
  }
  if (QString(Constants::SENTRY_DSN_ENDPOINT).isEmpty() ||
      QString(Constants::SENTRY_ENVELOPE_INGESTION).isEmpty()) {
    logger.error() << "Sentry failed to init, no sentry config present";
    return;
  }

  auto log = LogHandler::instance();

  QDir dataDir(
      QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
  QString sentryFolder = dataDir.absoluteFilePath("sentry");

  connect(qApp, &QCoreApplication::aboutToQuit, this,
          &SentryAdapter::onBeforeShutdown);
  connect(log, &LogHandler::logEntryAdded, this,
          &SentryAdapter::onLoglineAdded);

  sentry_options_t* options = sentry_options_new();
  sentry_options_set_dsn(options, Constants::SENTRY_DSN_ENDPOINT);
  sentry_options_set_environment(
      options, Constants::inProduction() ? "production" : "stage");
  sentry_options_set_release(
      options, Constants::versionString().toLocal8Bit().constData());
  sentry_options_set_database_path(options,
                                   sentryFolder.toLocal8Bit().constData());
  sentry_options_set_on_crash(options, &SentryAdapter::onCrash, NULL);

#ifdef MZ_IOS
  sentry_set_tag("os.name", "iOS");
  sentry_set_tag("os.version", QSysInfo::productVersion().toLocal8Bit().constData());
#endif

#ifdef SENTRY_NONE_TRANSPORT
  sentry_transport_t* transport =
      sentry_transport_new(&SentryAdapter::transportEnvelope);
  sentry_transport_set_state(transport, nullptr);
  sentry_options_set_transport(options, transport);
#endif

  // Uncomment the following line for debugging purposes.
  // Be warned, it's spammy to stdout.
  // sentry_options_set_debug(options, 1);

  if (sentry_init(options) == -1) {
    logger.error() << "Sentry failed to init!";
    return;
  };
  m_initialized = true;
  logger.info() << "Sentry initialized";
}

void SentryAdapter::report(const QString& errorType, const QString& message,
                           bool attachStackTrace) {
  if (!m_initialized) {
    return;
  }
  sentry_value_t event = sentry_value_new_event();
  sentry_value_t exc = sentry_value_new_exception(errorType.toLocal8Bit(),
                                                  message.toLocal8Bit());

  if (attachStackTrace) {
    sentry_value_set_stacktrace(exc, NULL, 0);
  }
  sentry_event_add_exception(event, exc);
  sentry_capture_event(event);
}

void SentryAdapter::onBeforeShutdown() { sentry_close(); }

void SentryAdapter::onLoglineAdded(const QByteArray& line) {
  if (!m_initialized) {
    return;
  }
  // Todo: we could certainly catch this more early and format the data ?
  // (VPN-3276)
  sentry_value_t crumb =
      sentry_value_new_breadcrumb("Logger", line.constData());
  sentry_add_breadcrumb(crumb);
}

sentry_value_t SentryAdapter::onCrash(const sentry_ucontext_t* uctx,
                                      sentry_value_t event, void* closure) {
  logger.info() << "Sentry ON CRASH";
  // Do contextual clean-up before the crash is sent to sentry's backend
  // infrastructure
  bool shouldSend = true;
  // Todo: We can use this callback to make sure
  // we only send data with user consent.
  // Tracked in: VPN-3158
  // We could:
  //  -> Maybe start a new Process for the Crash-Report UI ask for consent
  //  (VPN-2823)
  //  -> Check if a setting "upload crashes" is present.
  // If we should not send it, we can discard the crash data here :)
  if (shouldSend) {
    return event;
  }
  sentry_value_decref(event);
  return sentry_value_new_null();
}

// static
void SentryAdapter::transportEnvelope(sentry_envelope_t* envelope,
                                      void* state) {
  Q_UNUSED(state);
  size_t sentry_buf_size = 0;
  char* sentry_buf = sentry_envelope_serialize(envelope, &sentry_buf_size);

  // Qt Will copy this.
  auto qt_owned_buffer = QByteArray(sentry_buf, sentry_buf_size);
  // We can now free the envelope.
  sentry_envelope_free(envelope);
  sentry_free(sentry_buf);

  auto t = new TaskSentry(qt_owned_buffer);
  TaskScheduler::scheduleTask(t);
}

SentryAdapter::UserConsentResult SentryAdapter::hasCrashUploadConsent() const {
  Q_ASSERT(m_initialized);
  // We have not yet asked the user - let's do that.
  if (m_userConsent == UserConsentResult::Pending) {
    emit needsCrashReportScreen();
  }
  return m_userConsent;
}
// Q_Invokeable
void SentryAdapter::allowCrashReporting() {
  m_userConsent = UserConsentResult::Allowed;
  emit userConsentChanged();
}
// Q_Invokeable
void SentryAdapter::declineCrashReporting() {
  m_userConsent = UserConsentResult::Forbidden;
  emit userConsentChanged();
}
