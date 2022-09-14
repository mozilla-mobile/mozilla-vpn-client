/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "sentryadapter.h"

#include <QObject>

namespace {
SentryAdapter* s_instance = nullptr;

}  // namespace
SentryAdapter::SentryAdapter() {}

SentryAdapter::~SentryAdapter() {}

SentryAdapter* SentryAdapter::instance() { 
    if (s_instance == nullptr) {
        s_instance = new SentryAdapter();
    }
    return s_instance;
}

void SentryAdapter::init() {}

void SentryAdapter::report(const QString& category, const QString& message,
                           bool attachStackTrace) {
  Q_UNUSED(category);
  Q_UNUSED(message);
  Q_UNUSED(attachStackTrace);
}