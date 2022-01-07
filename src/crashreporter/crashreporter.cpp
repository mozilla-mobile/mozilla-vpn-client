/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "crashreporter.h"
#include <iostream>
#include "crashui.h"

using namespace std;

CrashReporter::CrashReporter(QObject* parent) : QObject(parent) {
  m_ui = make_unique<CrashUI>();
}

bool CrashReporter::shouldPromptUser() {
  // eventually this will need to check settings for an opt-in.  For now we
  // always ask
  return true;
}

bool CrashReporter::promptUser() {
  m_ui->initialize();
  connect(m_ui.get(), &CrashUI::startUpload, this,
          [this]() { emit startUpload(); });
  connect(m_ui.get(), &CrashUI::cleanupDumps, this,
          [this]() { emit cleanup(); });
  m_ui->showUI();
  return true;
}
