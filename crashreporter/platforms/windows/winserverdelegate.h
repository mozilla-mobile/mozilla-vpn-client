#ifndef WINSERVERDELEGATE_H
#define WINSERVERDELEGATE_H

#include <util/win/exception_handler_server.h>
#include <QObject>

class WinServerDelegate : public crashpad::ExceptionHandlerServer::Delegate, QObject
{
public:
    WinServerDelegate();
    unsigned int ExceptionHandlerServerException(HANDLE process, crashpad::WinVMAddress exception_information_address, crashpad::WinVMAddress debug_critical_section_address) override;
    void ExceptionHandlerServerStarted() override;
};

#endif // WINSERVERDELEGATE_H
