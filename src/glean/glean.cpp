/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "extern.h"
#include "glean.h"
#include "settingsholder.h"

#if defined(MVPN_IOS)
#  include "platforms/ios/iosgleanglue.h"
#endif

// static
void Glean::initialize() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

#if defined(MVPN_IOS)
  new IOSGleanGlue(settingsHolder->gleanEnabled());
#else
  glean_initialize(settingsHolder->gleanEnabled());
#endif
}
