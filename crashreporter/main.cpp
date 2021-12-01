#include <QCoreApplication>

#include <QTimer>
#include "crashreporterfactory.h"
#include <qsystemdetection.h>
#include <iostream>
#if defined Q_OS_WIN && defined MVPN_DEBUG
#  include <windows.h>
#endif

int main(int argc, char *argv[])
{
#ifdef MVPN_DEBUG
  #ifdef Q_OS_WIN
  // Allocate a console to view log output in debug mode on windows
  if (AllocConsole()) {
    FILE* unusedFile;
    freopen_s(&unusedFile, "CONOUT$", "w", stdout);
    freopen_s(&unusedFile, "CONOUT$", "w", stderr);
    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
  }
#endif
#endif
    QCoreApplication a(argc, argv);
    QTimer::singleShot(0, [](){
        auto crashreporter = CrashReporterFactory::createCrashReporter();
        crashreporter->start();
    });
    return a.exec();
}
