/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "sentry/sentryadapter.h"

SentryAdapter* SentryAdapter::instance() { return nullptr; }
SentryAdapter::SentryAdapter() {}
SentryAdapter::~SentryAdapter() {}

void SentryAdapter::init() {}

void SentryAdapter::report(const QString& errorType, const QString& message,
                           bool attachStackTrace) {
  Q_UNUSED(errorType);
  Q_UNUSED(message);
  Q_UNUSED(attachStackTrace);
}

void SentryAdapter::onBeforeShutdown() {}

void SentryAdapter::captureQMLStacktrace(const char* description) {
  Q_UNUSED(description);
}

void SentryAdapter::onLoglineAdded(const QByteArray& line) { Q_UNUSED(line); }

sentry_value_t SentryAdapter::onCrash(const sentry_ucontext_t* uctx,
                                      sentry_value_t event, void* closure) {
  Q_UNUSED(uctx);
  Q_UNUSED(event);
  Q_UNUSED(closure);
  return nullptr;
}

// static
void SentryAdapter::transportEnvelope(sentry_envelope_t* envelope,
                                      void* state) {
  Q_UNUSED(envelope);
  Q_UNUSED(state);
}

SentryAdapter::UserConsentResult SentryAdapter::hasCrashUploadConsent() const {
  return UserConsentResult::Pending;
}
// Q_Invokeable
void SentryAdapter::allowCrashReporting() {}
// Q_Invokeable
void SentryAdapter::declineCrashReporting() {}
