#include "windowscrashserverclient.h"

#include <QObject>
#include <iostream>
#include <codecvt>
#include <util/win/registration_protocol_win.h>
#include <util/win/get_function.h>
#include "../../crashconstants.h"
#include "../logger.h"

using namespace crashpad;
using namespace std;

namespace {
Logger logger("CrashClient", "WindowsCrashServerClient");
}

WindowsCrashServerClient::WindowsCrashServerClient() {
  m_client = make_unique<CrashpadClient>();
}

WindowsCrashServerClient::~WindowsCrashServerClient() {
  ClientToServerMessage message = {};
  message.type = ClientToServerMessage::kShutdown;
  ServerToClientMessage response = {};
}

bool WindowsCrashServerClient::start(int argc, char* argv[]) {
  vector<string> args{"--crashreporter"};
  wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  wstring path = converter.from_bytes(argv[0]);
  return m_client->StartHandler(base::FilePath(path), base::FilePath(),
                                base::FilePath(), "", {}, args, false, true,
                                {});
}
