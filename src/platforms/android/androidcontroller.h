/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDCONTROLLER_H
#define ANDROIDCONTROLLER_H

#include "controllerimpl.h"
#include "models/device.h"

#include <QAndroidBinder>
#include <QAndroidServiceConnection>

class AndroidController final : public ControllerImpl,
                                public QAndroidServiceConnection {
  Q_DISABLE_COPY_MOVE(AndroidController)

 public:
  AndroidController();
  static AndroidController* instance();
  ~AndroidController();

  // from ControllerImpl
  void initialize(const Device* device, const Keys* keys) override;

  void activate(const Server& data, const Device* device, const Keys* keys,
                const QList<IPAddressRange>& allowedIPAddressRanges,
                const QList<QString>& vpnDisabledApps, const QHostAddress& dns,
                Reason reason) override;
  void resume_activate();

  void deactivate(Reason reason) override;

  void checkStatus() override;

  void setNotificationText(const QString& title, const QString& message,
                           int timerSec);
  void setFallbackConnectedNotification();

  void getBackendLogs(std::function<void(const QString&)>&& callback) override;

  void cleanupBackendLogs() override;

  // from QAndroidServiceConnection
  void onServiceConnected(const QString& name,
                          const QAndroidBinder& serviceBinder) override;
  void onServiceDisconnected(const QString& name) override;

 private:
  Server m_server;
  Device m_device;
  bool m_serviceConnected = false;
  std::function<void(const QString&)> m_logCallback;

  QAndroidBinder m_serviceBinder;
  class VPNBinder : public QAndroidBinder {
   public:
    VPNBinder(AndroidController* controller) : m_controller(controller) {}

    bool onTransact(int code, const QAndroidParcel& data,
                    const QAndroidParcel& reply,
                    QAndroidBinder::CallType flags) override;

    QString readUTF8Parcel(QAndroidParcel data);

   private:
    AndroidController* m_controller = nullptr;
  };

  VPNBinder m_binder;

  static void startActivityForResult(JNIEnv* env, jobject /*thiz*/,
                                     jobject intent);
};

#endif  // ANDROIDCONTROLLER_H
