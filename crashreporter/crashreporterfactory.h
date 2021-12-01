#ifndef CRASHREPORTERFACTORY_H
#define CRASHREPORTERFACTORY_H

#include <memory>
#include "crashreporter.h"

class CrashReporterFactory
{
public:
    CrashReporterFactory();
    static std::shared_ptr<CrashReporter> createCrashReporter();
};

#endif // CRASHREPORTERFACTORY_H
