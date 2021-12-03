/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "winserverdelegate.h"
#include "wincrashdata.h"

using namespace crashpad;

WinServerDelegate::WinServerDelegate()
{

}

unsigned int WinServerDelegate::ExceptionHandlerServerException(HANDLE process, WinVMAddress exception_information_address, WinVMAddress debug_critical_section_address){
    WinCrashData data(process, exception_information_address, debug_critical_section_address);
    emit crashReported(data);
    return 0;
}

void WinServerDelegate::ExceptionHandlerServerStarted(){

}
