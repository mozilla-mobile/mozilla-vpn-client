#ifndef MACOSSWIFTCONTROLLER_H
#define MACOSSWIFTCONTROLLER_H

#include "controller.h"

#include <QString>
#include <functional>

class Keys;
class Device;
class Server;
class QDateTime;

class MacOSSwiftController
{
public:
    static void initialize(const Device *device,
                           const Keys *keys,
                           std::function<void(bool, Controller::State, const QDateTime&)>&& callback,
                           std::function<void(Controller::State)>&& externalCallback);

    static void activate(const Server* server, std::function<void()>&& failureCallback);

    static void deactivate();
};

#endif // MACOSSWIFTCONTROLLER_H
