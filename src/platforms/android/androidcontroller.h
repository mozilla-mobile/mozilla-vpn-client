/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDCONTROLLER_H
#define ANDROIDCONTROLLER_H

#include "controllerimpl.h"
#include <QAndroidBinder>
#include <QAndroidServiceConnection>

class AndroidController: public ControllerImpl, public QAndroidServiceConnection
{
public:
    // from ControllerImpl
    void initialize(const Device *device, const Keys *keys) override;

    void activate(const Server &data,
                  const Device *device,
                  const Keys *keys,
                  const CaptivePortal &captivePortal,
                  bool forSwitching) override;

    void deactivate(bool forSwitching) override;

    void checkStatus() override;

    void onRecviceConnected();
    void onRecviceDisconnected();

    void getBackendLogs(std::function<void(const QString &)> &&callback) override;

    // from QAndroidServiceConnection
    void onServiceConnected(const QString &name, const QAndroidBinder &serviceBinder) override;
    void onServiceDisconnected(const QString &name) override;
private:
    int64_t m_txBytes = 0;
    int64_t m_rxBytes = 0;

    QAndroidBinder m_serviceBinder;
    bool GetVpnPermission();
    class VPNBinder : public QAndroidBinder{
    public:
         void setController(AndroidController*);
         bool onTransact(int code, const QAndroidParcel &data, const QAndroidParcel &reply, QAndroidBinder::CallType flags) override;

    private:
         AndroidController* mController;
    };

    VPNBinder m_binder;
};

#endif // DUMMYCONTROLLER_H
