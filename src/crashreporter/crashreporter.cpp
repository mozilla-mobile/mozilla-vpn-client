/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "crashreporter.h"

CrashReporter::CrashReporter(QObject* parent) : QObject(parent) {}

bool CrashReporter::shouldPromptUser() {
  // eventually this will need to check settings for an opt-in.  For now we
  // always ask
  return true;
}

void CrashReporter::crashReported(const CrashData& data) {}
