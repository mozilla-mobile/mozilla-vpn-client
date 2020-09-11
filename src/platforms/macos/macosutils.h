#ifndef MACOSUTILS_H
#define MACOSUTILS_H

#include <QString>
#include <functional>

class Keys;
class Device;

class MacosUtils
{
public:
    static QString computerName();

    static void maybeInitializeController(const Device *device,
                                          const Keys *keys,
                                          std::function<void(bool)>&& callback);

    static void controllerActivate(std::function<void(bool)>&& callback);
};

#endif // MACOSUTILS_H
