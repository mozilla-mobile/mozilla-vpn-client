/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wincrashdata.h"
#include <iostream>

using namespace crashpad;

WinCrashData::WinCrashData(HANDLE procHandle, crashpad::WinVMAddress exception_information_address, crashpad::WinVMAddress debug_critical_section_address) : CrashData(), m_handle(procHandle), m_exceptAddress(exception_information_address), m_debugAddress(debug_critical_section_address) {
  std::cout << "Process handle is: " << procHandle;
}

HANDLE WinCrashData::getHandle(){
    return m_handle;
}

WinVMAddress WinCrashData::getDebugCritSectionAddress(){
    return m_debugAddress;
}

WinVMAddress WinCrashData::getExceptionInfoAddress(){
    return m_exceptAddress;
}
