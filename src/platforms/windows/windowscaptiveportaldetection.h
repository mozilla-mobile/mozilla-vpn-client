/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSCAPTIVEPORTALDETECTION_H
#define WINDOWSCAPTIVEPORTALDETECTION_H

#include "captiveportal/captiveportaldetectionimpl.h"

#include <QThread>

class WindowsCaptivePortalDetection final : public CaptivePortalDetectionImpl {
  Q_OBJECT

 public:
  WindowsCaptivePortalDetection();
  ~WindowsCaptivePortalDetection();

  void start() override;

 signals:
  void startWorker(const QString& ip, const QString& host, const QString& url,
                   const QString& expectedResult);

 private:
  QThread m_thread;
};

#endif  // WINDOWSCAPTIVEPORTALDETECTION_H
