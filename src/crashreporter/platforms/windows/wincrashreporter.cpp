/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wincrashreporter.h"
#include "crashreporter/crashuploader.h"

#include <iostream>
#include <QStandardPaths>
#include <Windows.h>
#include <QDir>

using namespace std;

WinCrashReporter::WinCrashReporter(QObject* parent) : CrashReporter(parent) {
  m_uploader = make_unique<CrashUploader>();
}

bool WinCrashReporter::start(int argc, char* argv[]) {
  if (AllocConsole()) {
    FILE* unusedFile;
    freopen_s(&unusedFile, "CONOUT$", "w", stdout);
    freopen_s(&unusedFile, "CONOUT$", "w", stderr);
    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
  }
  // On windows, all we do is check with the user and upload the crashdump if
  // allowed
  if (shouldPromptUser()) {
    if (!promptUser()) {
      return false;
    }
  }

  // QStringList appDatas =
  //    QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
  // QString appLocal = appDatas.first() + "\\dumps";
  // QDir dumpRoot(appLocal);
  // QStringList filters = {"*.dmp"};
  // dumpRoot.setNameFilters(filters);
  // QStringList dumpFiles = dumpRoot.entryList();
  // QStringList absDumpPaths;
  // for (auto file : dumpFiles) {
  //  absDumpPaths << dumpRoot.absoluteFilePath(file);
  //}

  // for (auto s : absDumpPaths) {
  //  cout << s.toStdString() << endl;
  //}

  // m_uploader->startUploads(absDumpPaths);
  return true;
}
