/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
