/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAPTIVEPORTALDETECTION_H
#define CAPTIVEPORTALDETECTION_H

#include "captiveportalnotifier.h"

class CaptivePortalDetectionImpl;

class CaptivePortalDetection final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(CaptivePortalDetection)

 public:
  CaptivePortalDetection();
  ~CaptivePortalDetection();

  void initialize();

 public slots:
  void stateChanged();
  void settingsChanged();
  void detectionCompleted(bool detected);
  void notificationCompleted(bool disconnectionRequested);

 private:
  bool m_active = false;

  CaptivePortalNotifier m_captivePortalNotifier;

  QScopedPointer<CaptivePortalDetectionImpl> m_impl;
};

#endif  // CAPTIVEPORTALDETECTION_H
