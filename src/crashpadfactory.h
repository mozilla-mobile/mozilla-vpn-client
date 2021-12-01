#ifndef CRASHPADFACTORY_H
#define CRASHPADFACTORY_H

#include <memory>

#include "crashpad.h"

class CrashpadFactory
{
public:
    CrashpadFactory();
    static std::shared_ptr<Crashpad> createCraskpad();
};

#endif // CRASHPADFACTORY_H
