/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wincrashreporter.h"
#include "crashreporter/crashuploader.h"
#include <QApplication>
#include <QStandardPaths>
#include <Windows.h>
#include <QDir>
#include <iostream>
#include "logger.h"

using namespace std;

namespace {
Logger logger(LOG_CRASHREPORTER, "WinCrashReporter");
}

WinCrashReporter::WinCrashReporter(QObject* parent) : CrashReporter(parent) {
  m_uploader = make_unique<CrashUploader>();
}

bool WinCrashReporter::start(int argc, char* argv[]) {
  Q_UNUSED(argc);
  Q_UNUSED(argv);
#ifdef MVPN_DEBUG
  if (AllocConsole()) {
    FILE* unusedFile;
    freopen_s(&unusedFile, "CONOUT$", "w", stdout);
    freopen_s(&unusedFile, "CONOUT$", "w", stderr);
    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
  }
#endif  // MVPN_DEBUG

  auto appDatas =
      QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
  auto appLocal = appDatas.first() + "\\dumps";
  QDir dumpRoot(appLocal);
  QStringList filters = {"*.dmp"};
  dumpRoot.setNameFilters(filters);
  QStringList absDumpPaths;

  // In case the dump has not been written, retry 5 times
  // with an increasing sleep interval
  for (int i = 1; i < 6; i++) {
    auto dumpFiles = dumpRoot.entryList();
    if (dumpFiles.empty()) {
      if (i < 5) {
        Sleep(i * 1000);
      }
    } else {
      for (auto file : dumpFiles) {
        absDumpPaths << dumpRoot.absoluteFilePath(file);
      }
      break;
    }
  }

  if (absDumpPaths.empty()) {
    logger.info() << "No dump files found.";
    return false;
  }
  connect(this, &CrashReporter::startUpload, this,
          [this, absDumpPaths]() { m_uploader->startUploads(absDumpPaths); });
  connect(this, &CrashReporter::cleanup, this,
          [this, absDumpPaths]() { cleanupDumps(absDumpPaths); });
  connect(m_uploader.get(), &CrashUploader::uploadsComplete, this,
          [this, absDumpPaths]() { cleanupDumps(absDumpPaths); });
  if (shouldPromptUser()) {
    if (!promptUser()) {
      return false;
    }
  } else {
    startUpload();
  }
  return true;
}

void WinCrashReporter::cleanupDumps(QStringList files) {
  for (auto file : files) {
    if (!QFile::remove(file)) {
      logger.error() << "Unable to delete dump file: " << file;
    }
  }
  qApp->quit();
}
