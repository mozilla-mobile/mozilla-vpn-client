/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "webupdater.h"

#include "glean/generated/metrics.h"
#include "leakdetector.h"
#include "logging/logger.h"
#include "task.h"
#include "urlopener.h"

namespace {
Logger logger("WebUpdater");
}

WebUpdater::WebUpdater(QObject* parent) : Updater(parent) {
  MZ_COUNT_CTOR(WebUpdater);
  logger.debug() << "WebUpdater created";
}

WebUpdater::~WebUpdater() {
  MZ_COUNT_DTOR(WebUpdater);
  logger.debug() << "WebUpdater released";
}

void WebUpdater::start(Task*) {
  mozilla::glean::sample::update_step.record(
      mozilla::glean::sample::UpdateStepExtra{
          ._state = QVariant::fromValue(FallbackInBrowser).toString()});

  UrlOpener::instance()->openUrlLabel("update");
  deleteLater();
}
