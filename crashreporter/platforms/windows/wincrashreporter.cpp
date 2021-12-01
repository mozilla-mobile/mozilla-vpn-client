#include "wincrashreporter.h"
#include "crashconstants.h"

using namespace std;
using namespace crashpad;

WinCrashReporter::WinCrashReporter(QObject *parent) : CrashReporter(parent)
{
    m_server = make_unique<ExceptionHandlerServer>(false);
}

bool WinCrashReporter::start(){
    m_server->SetPipeName(IPC_NAME);
    WinServerDelegate * delegate = new WinServerDelegate();
    connect(delegate, &WinServerDelegate::crashReported, this, &WinCrashReporter::crashReported);
    //run takes ownership of the delegate pointer
    m_server->Run(delegate);
    return true;
}
