/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAPTIVEPORTALDETECTION_H
#define CAPTIVEPORTALDETECTION_H

#include <QObject>
#include "captiveportalresult.h"

class CaptivePortalMonitor;
class CaptivePortalNotifier;

class CaptivePortalDetection final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(CaptivePortalDetection)

 public:
  CaptivePortalDetection();
  ~CaptivePortalDetection();

  void initialize();

  // Methods exposed for the inspector.
  void detectCaptivePortal();
  void captivePortalDetected();

 public slots:
  void stateChanged();
  void settingsChanged();
  void detectionCompleted(CaptivePortalResult detected);
  void captivePortalGone();

  void activationRequired();
  void deactivationRequired();

 private:
  CaptivePortalMonitor* captivePortalMonitor();
  CaptivePortalMonitor* captivePortalBackgroundMonitor();

  CaptivePortalNotifier* captivePortalNotifier();

 private:
  bool m_active = false;
  bool m_shouldRun = true;

  // Don't use it directly. Use captivePortalMonitor().
  CaptivePortalMonitor* m_captivePortalMonitor = nullptr;

  CaptivePortalMonitor* m_captivePortalBackgroundMonitor = nullptr;

  // Don't use it directly. Use captivePortalNotifier().
  CaptivePortalNotifier* m_captivePortalNotifier = nullptr;
};

#endif  // CAPTIVEPORTALDETECTION_H
