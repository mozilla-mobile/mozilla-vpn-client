/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowscaptiveportaldetection.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "windowscaptiveportaldetectionthread.h"
#include "../captiveportal/captiveportalresult.h"

#include <QScopeGuard>

namespace {
Logger logger(LOG_WINDOWS, "WindowsCaptivePortalDetection");
}

WindowsCaptivePortalDetection::WindowsCaptivePortalDetection() {
  MVPN_COUNT_CTOR(WindowsCaptivePortalDetection);

  m_thread.start();
}

WindowsCaptivePortalDetection::~WindowsCaptivePortalDetection() {
  MVPN_COUNT_DTOR(WindowsCaptivePortalDetection);

  m_thread.quit();
  m_thread.wait();
}

void WindowsCaptivePortalDetection::start() {
  logger.log() << "Captive portal detection started";

  auto guard = qScopeGuard(
      [&] { emit detectionCompleted(CaptivePortalResult::NoPortal); });

  QStringList ipv4Addresses =
      MozillaVPN::instance()->captivePortal()->ipv4Addresses();
  if (ipv4Addresses.isEmpty()) {
    logger.log() << "No ipv4 addresses for the captive portal endpoint";
    return;
  }

  WindowsCaptivePortalDetectionThread* thread =
      new WindowsCaptivePortalDetectionThread(this);
  thread->moveToThread(&m_thread);

  connect(&m_thread, &QThread::finished, thread, &QObject::deleteLater);
  connect(this, &QObject::destroyed, thread, &QObject::deleteLater);
  connect(thread, &WindowsCaptivePortalDetectionThread::detectionCompleted,
          [this](bool detected) {
            emit detectionCompleted(detected ? PortalDetected : NoPortal);
          });
  connect(this, &WindowsCaptivePortalDetection::startWorker, thread,
          &WindowsCaptivePortalDetectionThread::startWorker);

  QString ipv4 = ipv4Addresses[0];
  QString url = QString(CAPTIVEPORTAL_URL_IPV4).arg(ipv4);

  emit startWorker(ipv4, CAPTIVEPORTAL_HOST, url,
                   CAPTIVEPORTAL_REQUEST_CONTENT);

  guard.dismiss();
}
