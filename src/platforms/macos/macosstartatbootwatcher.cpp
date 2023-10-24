/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosstartatbootwatcher.h"

#include "leakdetector.h"
#include "logger.h"
#include "platforms/macos/macosutils.h"
#include "settingsholder.h"

namespace {
Logger logger("MacOSStartAtBootWatcher");
}

MacOSStartAtBootWatcher::MacOSStartAtBootWatcher() {
  MZ_COUNT_CTOR(MacOSStartAtBootWatcher);

  logger.debug() << "StartAtBoot watcher";

  connect(SettingsHolder::instance(), &SettingsHolder::startAtBootChanged, this,
          &MacOSStartAtBootWatcher::startAtBootChanged);

  startAtBootChanged();
}

MacOSStartAtBootWatcher::~MacOSStartAtBootWatcher() {
  MZ_COUNT_DTOR(MacOSStartAtBootWatcher);
}

void MacOSStartAtBootWatcher::startAtBootChanged() {
  bool startAtBoot = SettingsHolder::instance()->startAtBoot();

  logger.debug() << "StartAtBoot changed:" << startAtBoot;
  MacOSUtils::enableLoginItem(startAtBoot);
}
