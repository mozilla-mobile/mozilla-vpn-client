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
                  bool forSwitching) override;

    void deactivate(bool forSwitching) override;

    void checkStatus() override;

private:
    void monitorWatcher(QDBusPendingCallWatcher *watcher);

private:
    DBus *m_dbus = nullptr;
};

#endif // LINUXCONTROLLER_H
