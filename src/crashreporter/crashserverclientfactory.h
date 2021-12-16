/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CRASHSERVERCLIENTFACTORY_H
#define CRASHSERVERCLIENTFACTORY_H

#include "crashserverclient.h"
#include <memory>

class CrashServerClientFactory
{
public:
    CrashServerClientFactory();
    static std::shared_ptr<CrashServerClient> create();
};

#endif // CRASHSERVERCLIENTFACTORY_H
