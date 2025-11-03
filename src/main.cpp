/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCoreApplication>

#include "commandlineparser.h"
#include "leakdetector.h"
#include "platforms/windows/windowsutils.h"
#include "stdio.h"
#include "version.h"

#ifdef MZ_WINDOWS
#  include <windows.h>

#  include <iostream>

#  include "platforms/windows/windowsutils.h"
#endif

#ifdef MVPN_WEBEXTENSION
#  include <QFileInfo>
#  include <QUrl>

#  include "webextension/webextcommand.h"
#endif

constexpr const char* CLP_DEFAULT_COMMAND = "ui";

Q_DECL_EXPORT int main(int argc, char* argv[]) {
#ifdef MZ_DEBUG
  LeakDetector leakDetector;
  Q_UNUSED(leakDetector);
#endif

#ifdef MZ_WINDOWS
  if (AttachConsole(ATTACH_PARENT_PROCESS) != 0) {
    FILE* unusedFile;
    // Swap to the new out/err streams
    freopen_s(&unusedFile, "CONOUT$", "w", stdout);
    freopen_s(&unusedFile, "CONOUT$", "w", stderr);
    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
  }
  WindowsUtils::lockDownDLLSearchPath();
#endif

  QCoreApplication::setApplicationName("Mozilla VPN");
  QCoreApplication::setOrganizationName("Mozilla");
  QCoreApplication::setApplicationVersion(APP_VERSION);

#ifdef MVPN_WEBEXTENSION
  // Special case - if the first argument is a path to a file named
  // 'mozillavpn.json', or a URL with a scheme of 'chrome-extension'
  // then launch ourselves as the native messaging bridge.
  static Command::RegistrationProxy<WebExtCommand> s_commandWebExt;
  if (argc > 1) {
    if ((QFileInfo(argv[1]).fileName() == "mozillavpn.json") ||
        (QUrl(argv[1]).scheme() == "chrome-extension")) {
      char** newargs = (char**)calloc(argc+1, sizeof(char*));
      newargs[0] = argv[0];
      newargs[1] = (char*)"webext";
      for (int i = 1; i < argc; i++) {
        newargs[i+1] = argv[i];
      }
      argc++;
      argv = newargs;
    }
  }
#endif

  CommandLineParser clp;
  return clp.parse(argc, argv, CLP_DEFAULT_COMMAND);
}
