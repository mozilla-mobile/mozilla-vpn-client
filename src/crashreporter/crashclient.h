#ifndef CRASHCLIENT_H
#define CRASHCLIENT_H

#include <client/crashpad_client.h>
#include <memory>

class CrashClient {
 public:
  bool start(char * appPath);
  static CrashClient& instance();

 private:
  CrashClient();
  std::unique_ptr<crashpad::CrashpadClient> m_client;
};

#endif  // CRASHCLIENT_H
