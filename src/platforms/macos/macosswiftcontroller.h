#ifndef MACOSSWIFTCONTROLLER_H
#define MACOSSWIFTCONTROLLER_H

#include <QString>
#include <functional>

class Keys;
class Device;

class MacOSSwiftController
{
public:
    static void maybeInitialize(const Device *device,
                                const Keys *keys,
                                std::function<void(bool)>&& callback);

    static void activate(std::function<void(bool)>&& callback);

    static void deactivate(std::function<void(bool)>&& callback);
};

#endif // MACOSSWIFTCONTROLLER_H
