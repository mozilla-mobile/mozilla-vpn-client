/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "sentryadapter.h"

#include <sentry.h>

#include <QDir>

#include "constants.h"
#include "leakdetector.h"
#include "loghandler.h"
#include "logger.h"
#include "settingsholder.h"
#include "mozillavpn.h"

namespace {
SentryAdapter* s_instance = nullptr;
Logger logger(LOG_MAIN, "Sentry");

}  // namespace

SentryAdapter* SentryAdapter::instance() {
  if (s_instance == nullptr) {
    s_instance = new SentryAdapter();
  }
  return s_instance;
}
SentryAdapter::SentryAdapter() { MVPN_COUNT_CTOR(SentryAdapter); }
SentryAdapter::~SentryAdapter() { MVPN_COUNT_DTOR(SentryAdapter); }

void SentryAdapter::init() {
  if (Constants::inProduction()) {
    // If we're not in Production let's just not enable this :)
    return;
  }
  // Okay so Lets INIT
  auto vpn = MozillaVPN::instance();
  auto log = LogHandler::instance();

  QDir dataDir(
      QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
  QString sentryFolder = dataDir.absoluteFilePath("sentry");

  connect(vpn, &MozillaVPN::aboutToQuit, this,
          &SentryAdapter::onBeforeShutdown);
  connect(log, &LogHandler::logEntryAdded, this,
          &SentryAdapter::onLoglineAdded);

  sentry_options_t* options = sentry_options_new();
  sentry_options_set_dsn(options, Constants::SENTRY_DER);
  sentry_options_set_environment(
      options, Constants::inProduction() ? "production" : "stage");
  sentry_options_set_release(
      options, Constants::versionString().toLocal8Bit().constData());
  sentry_options_set_database_path(options,
                                   sentryFolder.toLocal8Bit().constData());
  sentry_options_set_on_crash(options, &SentryAdapter::onCrash, NULL);

  // Leaving this for convinence, be warned, it's spammy to stdout.
  // sentry_options_set_debug(options, 1);

  if (sentry_init(options) == -1) {
    logger.error() << "Sentry failed to init!";
    return;
  };
  logger.info() << "Sentry initialised";
}

void SentryAdapter::report(const QString& errorType, const QString& message,
                           bool attachStackTrace) {
  sentry_value_t event = sentry_value_new_event();
  sentry_value_t exc = sentry_value_new_exception(errorType.toLocal8Bit(),
                                                  message.toLocal8Bit());

  if (attachStackTrace) {
    sentry_value_set_stacktrace(exc, NULL, 0);
  }
  sentry_event_add_exception(event, exc);
  sentry_capture_event(event);
}

void SentryAdapter::onBeforeShutdown() {
  // Flush everything,
  sentry_close();
}

void SentryAdapter::onLoglineAdded(const QByteArray& line) {
  // Todo: we could certainly catch this more early and format the data ?
  sentry_value_t crumb =
      sentry_value_new_breadcrumb("Logger", line.constData());
  sentry_add_breadcrumb(crumb);
}

sentry_value_t SentryAdapter::onCrash(
    const sentry_ucontext_t*
        uctx,              // provides the user-space context of the crash
    sentry_value_t event,  // used the same way as in `before_send`
    void* closure  // user-data that you can provide at configuration time
) {
  logger.info() << "Sentry ON CRASH";
  // Do contextual clean-up before the crash is sent to sentry's backend
  // infrastructure
  bool shouldSend = true;
  // Todo: We can use this callback to make sure
  // we only send data with user consent.
  // We could:
  //  -> Maybe start a new Process for the Crash-Report UI ask for consent
  //  -> Check if a setting "upload crashes" is present.
  // If we should not send it, we can discard the crash data here :)
  if (shouldSend) {
    return event;
  }
  sentry_value_decref(event);
  return sentry_value_new_null();
}
