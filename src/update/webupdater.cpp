/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "webupdater.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "task.h"
#include "telemetry/gleansample.h"

namespace {
Logger logger(LOG_NETWORKING, "WebUpdater");
}

WebUpdater::WebUpdater(QObject* parent) : Updater(parent) {
  MVPN_COUNT_CTOR(WebUpdater);
  logger.debug() << "WebUpdater created";
}

WebUpdater::~WebUpdater() {
  MVPN_COUNT_DTOR(WebUpdater);
  logger.debug() << "WebUpdater released";
}

void WebUpdater::start(Task*) {
  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
      GleanSample::updateStep,
      {{"state", QVariant::fromValue(FallbackInBrowser).toString()}});

  vpn->openLink(MozillaVPN::LinkUpdate);
  deleteLater();
}
