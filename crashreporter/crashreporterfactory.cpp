#include "crashreporterfactory.h"

#include <qsystemdetection.h>

#ifdef Q_OS_WIN
    #include "platforms/windows/wincrashreporter.h"
#endif

using namespace std;

CrashReporterFactory::CrashReporterFactory()
{

}

shared_ptr<CrashReporter> CrashReporterFactory::createCrashReporter(){
#ifdef Q_OS_WIN
    return make_shared<WinCrashReporter>();
#endif
    return nullptr;
}
