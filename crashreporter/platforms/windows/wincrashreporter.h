#ifndef WINCRASHREPORTER_H
#define WINCRASHREPORTER_H

#include "crashreporter.h"
#include "winserverdelegate.h"

#include <util/win/exception_handler_server.h>

class WinCrashReporter : public CrashReporter
{
public:
    explicit WinCrashReporter(QObject *parent = nullptr);
    bool start() override;

private:
    std::unique_ptr<crashpad::ExceptionHandlerServer> m_server;
    std::weak_ptr<WinServerDelegate> m_delegate;
};

#endif // WINCRASHREPORTER_H
