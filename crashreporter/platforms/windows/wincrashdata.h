#ifndef WINCRASHDATA_H
#define WINCRASHDATA_H

#include "crashdata.h"

#include <util/win/address_types.h>
#include <Windows.h>

class WinCrashData : public CrashData
{
public:
    explicit WinCrashData(HANDLE procHandle, crashpad::WinVMAddress exception_information_address, crashpad::WinVMAddress debug_critical_section_address);
    HANDLE getHandle();
    crashpad::WinVMAddress getExceptionInfoAddress();
    crashpad::WinVMAddress getDebugCritSectionAddress();
private:
    HANDLE m_handle;
    crashpad::WinVMAddress m_exceptAddress;
    crashpad::WinVMAddress m_debugAddress;

};

#endif // WINCRASHDATA_H
