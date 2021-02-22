/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSCAPTIVEPORTALDETECTIONTHREAD_H
#define WINDOWSCAPTIVEPORTALDETECTIONTHREAD_H

#include <QObject>

class WindowsCaptivePortalDetectionThread final : public QObject {
  Q_OBJECT

 public:
  WindowsCaptivePortalDetectionThread(QObject* parent);
  ~WindowsCaptivePortalDetectionThread();

 signals:
  void detectionCompleted(bool detected);

 public:
  void startWorker(const QString& ip, const QString& host, const QString& url,
                   const QString& expectedResult);
};

#endif  // WINDOWSCAPTIVEPORTALDETECTIONTHREAD_H
