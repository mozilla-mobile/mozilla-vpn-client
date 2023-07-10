/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAPTIVEPORTALREQUEST_H
#define CAPTIVEPORTALREQUEST_H

#include <QObject>
#include <QUrl>

class Task;

class CaptivePortalRequest final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(CaptivePortalRequest)

 public:
  enum CaptivePortalResult { NoPortal, PortalDetected, Failure };
  Q_ENUM(CaptivePortalResult);

  explicit CaptivePortalRequest(Task* parent);
  ~CaptivePortalRequest();

  void run();

 signals:
  void completed(CaptivePortalRequest::CaptivePortalResult detected);

 private:
  void createRequest(const QUrl& url);
  void nextStep();
  void onResult(CaptivePortalResult portalDetected);

 private:
  int m_running = 0;
  int m_success = 0;
};

#endif  // CAPTIVEPORTALREQUEST_H
