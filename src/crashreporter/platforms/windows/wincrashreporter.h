/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINCRASHREPORTER_H
#define WINCRASHREPORTER_H

#include "../../crashreporter.h"
#include "crashreporter/crashuploader.h"

#include <memory>

class WinCrashReporter : public CrashReporter {
 public:
  explicit WinCrashReporter(QObject* parent = nullptr);
  bool start(int argc, char* argv[]) override;

 private:
  std::unique_ptr<CrashUploader> m_uploader;
};

#endif  // WINCRASHREPORTER_H
