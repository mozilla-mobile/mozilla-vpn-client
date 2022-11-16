/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SENTRYADAPTER_H
#define SENTRYADAPTER_H

#include <QObject>
#include <QApplication>

#ifdef SENTRY_ENABLED
#  include <sentry.h>
#endif

class SentryAdapter final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(SentryAdapter)

 public:
  ~SentryAdapter();
  static SentryAdapter* instance();

  // Inits Sentry.
  // In case Telemetry is disabled this is a no-op.
  void init();

  // Reports an Error - can attach extra data and a stack trace if needed.
  void report(const QString& category, const QString& message,
              bool attachStackTrace = false);

  // Called when a Line is added, will add this as a breadcrumb
  Q_SLOT void onLoglineAdded(const QByteArray& line);
  // Called before shutdown, will flush pending events.
  Q_SLOT void onBeforeShutdown();
#ifdef SENTRY_ENABLED
  // Only define Sentry related callbacks if we have the typedefs :)
  // Called before Sentry will send a crash report
  static sentry_value_t onCrash(const sentry_ucontext_t* uctx,
                                sentry_value_t event, void* closure);

  // Will be used if NONE_TRANSPORT is enabled
  // in that case this will be called if the client has a
  // report to send to Sentry.
  static void transportEnvelope(sentry_envelope_t* envelope, void* state);

#endif

 private:
  bool m_initialized = false;
  SentryAdapter();
};
#endif  // SENTRYADAPTER_H
