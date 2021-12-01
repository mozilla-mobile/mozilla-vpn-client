#include "winserverdelegate.h"
#include "wincrashdata.h"

using namespace crashpad;

WinServerDelegate::WinServerDelegate() : Qobject(nullptr)
{

}

unsigned int WinServerDelegate::ExceptionHandlerServerException(HANDLE process, WinVMAddress exception_information_address, WinVMAddress debug_critical_section_address){
    WinCrashData data(process, exception_information_address, debug_critical_section_address);
    emit crashReported(data);
    return 0;
}

void WinServerDelegate::ExceptionHandlerServerStarted(){

}
