/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LINUXCONTROLLER_H
#define LINUXCONTROLLER_H

#include "controllerimpl.h"

class DBus;
class QDBusPendingCallWatcher;

class LinuxController final : public ControllerImpl
{
public:
    LinuxController();

    void initialize(const Device *device, const Keys *keys) override;

    void activate(const Server &server,
                  const Device *device,
                  const Keys *keys,
                  const CaptivePortal &captivePortal,
                  bool forSwitching) override;

    void deactivate(bool forSwitching) override;

    void checkStatus() override;

    void getBackendLogs(std::function<void(const QString &)> &&callback) override;

private:
    void monitorWatcher(QDBusPendingCallWatcher *watcher);

private:
    DBus *m_dbus = nullptr;
};

#endif // LINUXCONTROLLER_H
