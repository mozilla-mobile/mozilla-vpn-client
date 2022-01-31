/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDCONTROLLER_H
#define ANDROIDCONTROLLER_H

#include "controllerimpl.h"
#include "models/device.h"

#if QT_VERSION >= 0x060000
#  include <QtCore/private/qandroidextras_p.h>
#else
#  include <QAndroidBinder>
#  include <QAndroidServiceConnection>
#endif

class AndroidController final : public ControllerImpl {
  Q_DISABLE_COPY_MOVE(AndroidController)

 public:
  AndroidController();
  static AndroidController* instance();
  ~AndroidController();

  // from ControllerImpl
  void initialize(const Device* device, const Keys* keys) override;

  void activate(const HopConnection& hop, const Device* device,
                const Keys* keys, Reason Reason) override;

  void resume_activate();

  void deactivate(Reason reason) override;

  void checkStatus() override;

  void getBackendLogs(std::function<void(const QString&)>&& callback) override;

  void cleanupBackendLogs() override;

 private:
  QString m_serverPublicKey;
  Device m_device;
  std::function<void(const QString&)> m_logCallback;

  static void startActivityForResult(JNIEnv* env, jobject /*thiz*/,
                                     jobject intent);
};

#endif  // ANDROIDCONTROLLER_H
