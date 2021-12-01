#ifndef WINCRASHPAD_H
#define WINCRASHPAD_H

#include <crashpad.h>

class WinCrashpad : public Crashpad
{
public:
    WinCrashpad();
    bool start() override;
};

#endif // WINCRASHPAD_H
