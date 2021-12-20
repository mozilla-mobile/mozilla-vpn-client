/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAPTIVEPORTALREQUESTTASK_H
#define CAPTIVEPORTALREQUESTTASK_H

#include "captiveportalrequest.h"
#include "task.h"

#include <QObject>

class CaptivePortalRequestTask final : public Task {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(CaptivePortalRequestTask)

 public:
  CaptivePortalRequestTask(bool retryOnFailure = true);
  ~CaptivePortalRequestTask();

  void run() override;

 signals:
  void operationCompleted(CaptivePortalRequest::CaptivePortalResult detected);

 private:
  void createRequest();
  void onResult(CaptivePortalRequest::CaptivePortalResult portalDetected);

 private:
  const bool m_retryOnFailure = true;
  bool m_completed = false;
};

#endif  // CAPTIVEPORTALREQUESTTASK_H
