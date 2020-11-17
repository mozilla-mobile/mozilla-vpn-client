/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDCONTROLLER_H
#define ANDROIDCONTROLLER_H

#include "controllerimpl.h"

#include <QAndroidBinder>
#include <QAndroidServiceConnection>

class AndroidController : public ControllerImpl, public QAndroidServiceConnection
{
public:
    AndroidController();
    static AndroidController *instance();
    ~AndroidController();

    // from ControllerImpl
    void initialize(const Device *device, const Keys *keys) override;

    void activate(const Server &data,
                  const Device *device,
                  const Keys *keys,
                  const QList<IPAddressRange> &allowedIPAddressRanges,
                  bool forSwitching) override;
    void resume_activate();

    void deactivate(bool forSwitching) override;

    void checkStatus() override;

    void enableStartAtBoot(bool enabled);

    void getBackendLogs(std::function<void(const QString &)> &&callback) override;

    // from QAndroidServiceConnection
    void onServiceConnected(const QString &name, const QAndroidBinder &serviceBinder) override;
    void onServiceDisconnected(const QString &name) override;

private:
    Server m_server;
    std::function<void(const QString &)> m_logCallback;

    QAndroidBinder m_serviceBinder;
    class VPNBinder : public QAndroidBinder
    {
    public:
        VPNBinder(AndroidController *controller) : m_controller(controller) {}

        bool onTransact(int code,
                        const QAndroidParcel &data,
                        const QAndroidParcel &reply,
                        QAndroidBinder::CallType flags) override;

    private:
        AndroidController *m_controller = nullptr;
    };

    VPNBinder m_binder;

    static void startActivityForResult(JNIEnv *env, jobject /*thiz*/, jobject intent);
};

#endif // ANDROIDCONTROLLER_H
