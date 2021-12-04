#include "crashclient.h"

#include <QCoreApplication>
#include <QString>
#include <vector>
#include <codecvt>

using namespace crashpad;
using namespace std;

CrashClient::CrashClient() { m_client = make_unique<CrashpadClient>(); }

CrashClient& CrashClient::instance() {
  static CrashClient instance;
  return instance;
}

bool CrashClient::start(char * appPath) {
  vector<string> args;
  args.push_back("--crashreporter");
  base::FilePath db;
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  base::FilePath handler(converter.from_bytes(appPath));
  if (!m_client->StartHandler(handler, db, db, "", {}, args, true, true)) {
    return false;
  }
  return true;
}
