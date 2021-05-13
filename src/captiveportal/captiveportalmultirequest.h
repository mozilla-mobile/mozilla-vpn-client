/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAPTIVEPORTALMULTIREQUEST_H
#define CAPTIVEPORTALMULTIREQUEST_H

#include "captiveportalresult.h"

#include <QObject>
#include <QUrl>
#include <QQueue>

class CaptivePortalMultiRequest final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(CaptivePortalMultiRequest)

 public:
  explicit CaptivePortalMultiRequest(QObject* parent);
  ~CaptivePortalMultiRequest();

  void run();

 signals:
  void completed(CaptivePortalResult detected);

 private:
  void createRequest();
  void onResult(CaptivePortalResult portalDetected);

 private:
  bool m_completed = false;
};

#endif  // CAPTIVEPORTALMULTIREQUEST_H
