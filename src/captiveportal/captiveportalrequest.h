/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAPTIVEPORTALREQUEST_H
#define CAPTIVEPORTALREQUEST_H

#include "captiveportalresult.h"

#include <QObject>
#include <QUrl>

class CaptivePortalRequest final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(CaptivePortalRequest)

 public:
  explicit CaptivePortalRequest(QObject* parent);
  ~CaptivePortalRequest();

  void run();

 signals:
  void completed(CaptivePortalResult detected);

 private:
  void createRequest(const QUrl& url);
  void nextStep();
  void onResult(CaptivePortalResult portalDetected);

 private:
  bool m_completed = false;
};

#endif  // CAPTIVEPORTALREQUEST_H
