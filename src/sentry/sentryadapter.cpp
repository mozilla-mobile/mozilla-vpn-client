/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "sentryadapter.h"

#include <sentry.h>

#include "constants.h"
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
SentryAdapter::SentryAdapter(){}
SentryAdapter::~SentryAdapter(){}

void SentryAdapter::init() {
    // Telemetry is disabled - so we should probably not send stuff
  /*#if (!SettingsHolder::instance()->gleanEnabled()) {
    return;
  }
  if (!Constants::inProduction()) {
    // Let's for now restrict this to non Production
    return;
  }*/
  // Okay so Lets INIT
  auto vpn = MozillaVPN::instance();
  auto log = LogHandler::instance();
   
  connect(vpn, &MozillaVPN::aboutToQuit, this, &SentryAdapter::onBeforeShutdown);
  connect(log, &LogHandler::logEntryAdded, this, &SentryAdapter::onLoglineAdded);
  logger.info() << "Sentry initialised";

  sentry_options_t *options = sentry_options_new();
  sentry_options_set_dsn(options, "https://6a476c1b57a34773a75c60036236a01d@o1396220.ingest.sentry.io/6719480");
  sentry_options_set_release(options,  Constants::versionString().toLocal8Bit().constData());
  //sentry_options_set_debug(options, 1);
  sentry_init(options);
}

void SentryAdapter::report(const QString& errorType, const QString& message,
                           bool attachStackTrace) {
  sentry_value_t event = sentry_value_new_event();
  sentry_value_t exc = sentry_value_new_exception(errorType.toLocal8Bit(), message.toLocal8Bit());

  if(attachStackTrace){
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
  //sentry_value_t crumb = sentry_value_new_breadcrumb("Logger", line.constData());
  //sentry_add_breadcrumb(crumb);
}