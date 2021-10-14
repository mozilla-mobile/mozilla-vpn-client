/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "captiveportalmonitor.h"
#include "captiveportalrequest.h"
#include "captiveportalresult.h"
#include "leakdetector.h"
#include "logger.h"

constexpr uint32_t CAPTIVE_PORTAL_MONITOR_MSEC = 10000;

namespace {
Logger logger(LOG_NETWORKING, "CaptivePortalMonitor");
}

CaptivePortalMonitor::CaptivePortalMonitor(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(CaptivePortalMonitor);

  connect(&m_timer, &QTimer::timeout, this, &CaptivePortalMonitor::check);
}

CaptivePortalMonitor::~CaptivePortalMonitor() {
  MVPN_COUNT_DTOR(CaptivePortalMonitor);
}

void CaptivePortalMonitor::start() {
  logger.debug() << "Captive portal monitor start";
  m_timer.start(CAPTIVE_PORTAL_MONITOR_MSEC);
}

void CaptivePortalMonitor::stop() {
  if (!m_timer.isActive()) {
    return;
  }
  logger.debug() << "Captive portal monitor stop";
  m_timer.stop();
}

void CaptivePortalMonitor::check() {
  logger.debug() << "Checking the internet connectivity";

  CaptivePortalRequest* request = new CaptivePortalRequest(this);
  connect(request, &CaptivePortalRequest::completed,
          [this](CaptivePortalResult result) {
            logger.debug() << "Captive portal detection:" << result;
            if (!m_timer.isActive()) {
              return;
            }
            if (result == CaptivePortalResult::Failure) {
              return;
            }
            if (result == CaptivePortalResult::PortalDetected) {
              emit offline();
              return;
            }
            // It seems that the captive-portal is gone.
            emit online();
          });

  request->run();
}
