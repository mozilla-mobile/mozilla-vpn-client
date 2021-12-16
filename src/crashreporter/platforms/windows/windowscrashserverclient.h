#ifndef WINDOWSCRASHSERVERCLIENT_H
#define WINDOWSCRASHSERVERCLIENT_H

#include "../../crashserverclient.h"
#include <client/crashpad_client.h>
#include <memory>

class WindowsCrashServerClient : public CrashServerClient {
 public:
  WindowsCrashServerClient();
  ~WindowsCrashServerClient();
  bool start(int args, char* argv[]) override;

 private:
  std::unique_ptr<crashpad::CrashpadClient> m_client;
};

#endif  // WINDOWSCRASHSERVERCLIENT_H
