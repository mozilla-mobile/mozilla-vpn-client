/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wincrashreporter.h"
#include "../../crashconstants.h"

using namespace std;
using namespace crashpad;

WinCrashReporter::WinCrashReporter(QObject *parent) : CrashReporter(parent)
{
    m_server = make_unique<ExceptionHandlerServer>(false);
}

bool WinCrashReporter::start(){
    m_server->SetPipeName(WIN_PIPE);
    WinServerDelegate * delegate = new WinServerDelegate();
    connect(delegate, &WinServerDelegate::crashReported, this, &WinCrashReporter::crashReported);
    //run takes ownership of the delegate pointer
    m_server->Run(delegate);
    return true;
}
