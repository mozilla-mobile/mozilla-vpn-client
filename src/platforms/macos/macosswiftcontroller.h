#ifndef MACOSSWIFTCONTROLLER_H
#define MACOSSWIFTCONTROLLER_H

#include <QString>
#include <functional>

class Keys;
class Device;

class MacOSSwiftController
{
public:
    static void maybeInitializeController(const Device *device,
                                          const Keys *keys,
                                          std::function<void(bool)>&& callback);

    static void controllerActivate(std::function<void(bool)>&& callback);
};

#endif // MACOSSWIFTCONTROLLER_H
