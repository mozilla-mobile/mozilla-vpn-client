/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "crashserverclientfactory.h"

#include <QObject>

#ifdef MVPN_WINDOWS
#include "platforms/windows/windowscrashclient.h"
#endif

using namespace std;

CrashServerClientFactory::CrashServerClientFactory()
{

}

shared_ptr<CrashServerClient> CrashServerClientFactory::create(){
#ifdef MVPN_WINDOWS
    return make_shared<WindowsCrashClient>();
#endif
    Q_ASSERT(false);
}
