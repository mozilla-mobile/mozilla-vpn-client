#ifndef MACOSSWIFTCONTROLLER_H
#define MACOSSWIFTCONTROLLER_H

#include "controller.h"

#include <QString>
#include <functional>

class Keys;
class Device;
class Server;

class MacOSSwiftController
{
public:
    static void initialize(const Device *device,
                           const Keys *keys,
                           std::function<void(bool, Controller::State)>&& callback,
                           std::function<void(Controller::State)>&& externalCallback);

    static void activate(const Server* server, std::function<void(bool)>&& callback);

    static void deactivate(std::function<void(bool)>&& callback);
};

#endif // MACOSSWIFTCONTROLLER_H
