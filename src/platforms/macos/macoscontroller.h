/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSCONTROLLER_H
#define MACOSCONTROLLER_H

#include "controllerimpl.h"
#include "localsocketcontroller.h"

class MacOSController final : public LocalSocketController {
  Q_DISABLE_COPY_MOVE(MacOSController)

 public:
  MacOSController();
  ~MacOSController();

  void initialize(const Device* device, const Keys* keys) override;

 private slots:
  void checkServiceStatus();

 private:
  void* m_service = nullptr;
  int m_smAppStatus;
  QTimer m_regTimer;
};

#endif  // MACOSCONTROLLER_H
