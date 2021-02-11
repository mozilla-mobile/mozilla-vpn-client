/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAPTIVEPORTALDETECTION_H
#define CAPTIVEPORTALDETECTION_H

#include "captiveportalmonitor.h"
#include "captiveportalnotifier.h"

class CaptivePortalDetectionImpl;

class CaptivePortalDetection final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(CaptivePortalDetection)

 public:
  CaptivePortalDetection();
  ~CaptivePortalDetection();

  void initialize();

  void detectCaptivePortal();

 public slots:
  void stateChanged();
  void settingsChanged();
  void detectionCompleted(bool detected);
  void notificationCaptivePortalBlockCompleted(bool disconnectionRequested);
  void notificationCaptivePortalUnblockCompleted(bool connectionRequested);
  void captivePortalGone();

 private:
  bool m_active = false;

  CaptivePortalMonitor m_captivePortalMonitor;
  CaptivePortalNotifier m_captivePortalNotifier;

  QScopedPointer<CaptivePortalDetectionImpl> m_impl;
};

#endif  // CAPTIVEPORTALDETECTION_H
