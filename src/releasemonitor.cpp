/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "releasemonitor.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "timersingleshot.h"
#include "update/updater.h"

namespace {
Logger logger(LOG_MAIN, "ReleaseMonitor");
}

ReleaseMonitor::ReleaseMonitor() {
  MVPN_COUNT_CTOR(ReleaseMonitor);

  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, this, &ReleaseMonitor::runInternal);
}

ReleaseMonitor::~ReleaseMonitor() { MVPN_COUNT_DTOR(ReleaseMonitor); }

void ReleaseMonitor::runSoon() {
  logger.log() << "ReleaseManager - Scheduling a quick timer";
  TimerSingleShot::create(this, 0, [this] { runInternal(); });
}

void ReleaseMonitor::runInternal() {
  logger.log() << "ReleaseMonitor started";

  Updater* updater = Updater::create(this, false);
  Q_ASSERT(updater);

  updater->start();
  connect(updater, &Updater::updateRequired, this,
          &ReleaseMonitor::updateRequired);
  connect(updater, &Updater::updateRecommended, this,
          &ReleaseMonitor::updateRecommended);
  connect(updater, &QObject::destroyed, this, &ReleaseMonitor::releaseChecked);
  connect(updater, &QObject::destroyed, this, &ReleaseMonitor::schedule);
}

void ReleaseMonitor::schedule() {
  logger.log() << "ReleaseMonitor scheduling";
  m_timer.start(Constants::releaseMonitorMsec());
}

void ReleaseMonitor::updateRequired() {
  logger.log() << "update required";
  MozillaVPN::instance()->setUpdateRecommended(false);
  MozillaVPN::instance()->controller()->updateRequired();
}

void ReleaseMonitor::updateRecommended() {
  logger.log() << "Update recommended";
  MozillaVPN::instance()->setUpdateRecommended(true);
}

void ReleaseMonitor::update() {
  logger.log() << "Update";

  Updater* updater = Updater::create(this, true);
  if (!updater) {
    logger.log() << "No updater supported for this platform. Fallback";

    MozillaVPN* vpn = MozillaVPN::instance();
    Q_ASSERT(vpn);

    vpn->openLink(MozillaVPN::LinkUpdate);
    vpn->setUpdating(false);
    return;
  }

  // The updater, in download mode, is not destroyed. So, if this happens,
  // probably something went wrong.
  connect(updater, &QObject::destroyed, [] {
    MozillaVPN* vpn = MozillaVPN::instance();
    Q_ASSERT(vpn);
    vpn->setUpdating(false);
  });

  updater->start();
}
