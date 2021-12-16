/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "crashclient.h"

#include <QCoreApplication>
#include <QString>
#include <util/win/registration_protocol_win.h>
#include "crashconstants.h"
#include "crashserverclientfactory.h"

using namespace crashpad;
using namespace std;

CrashClient::CrashClient() { m_client = CrashServerClientFactory::create(); }

CrashClient& CrashClient::instance() {
  static CrashClient instance;
  return instance;
}

bool CrashClient::start(int argc, char* argv[]) {
  return m_client->start(argc, argv);
}
