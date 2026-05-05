/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#ifdef SENTRY_ENABLED

#  include <QQmlEngine>

#  include "sentry/sentryadapter.h"

struct MZCrashReporter {
  Q_GADGET
  QML_FOREIGN(SentryAdapter)
  QML_NAMED_ELEMENT(MZCrashReporter)
  QML_SINGLETON

 public:
  static SentryAdapter* create(QQmlEngine*, QJSEngine*) {
    return SentryAdapter::instance();
  }
};

#endif  // SENTRY_ENABLED
