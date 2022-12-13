/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAPTIVEPORTALDETECTIONIMPL_H
#define CAPTIVEPORTALDETECTIONIMPL_H

#include <QObject>

#include "captiveportalrequest.h"

class CaptivePortalDetectionImpl : public QObject {
  Q_OBJECT

 public:
  CaptivePortalDetectionImpl();
  virtual ~CaptivePortalDetectionImpl();

  virtual void start();

 signals:
  void detectionCompleted(
      CaptivePortalRequest::CaptivePortalResult captivePortalDetected);
};

#endif  // CAPTIVEPORTALDETECTIONIMPL_H
