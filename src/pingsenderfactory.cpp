/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "pingsenderfactory.h"

#if defined(MVPN_LINUX) || defined(MVPN_ANDROID)
#  include "platforms/linux/linuxpingsender.h"
#elif defined(MVPN_MACOS) || defined(MVPN_IOS)
#  include "platforms/macos/macospingsender.h"
#elif defined(MVPN_WINDOWS)
#  include "platforms/windows/windowspingsender.h"
#elif defined(MVPN_DUMMY) || defined(UNIT_TEST)
#  include "platforms/dummy/dummypingsender.h"
#else
#  error "Unsupported platform"
#endif

PingSender* PingSenderFactory::create(const QHostAddress& source,
                                      QObject* parent) {
#if defined(MVPN_LINUX) || defined(MVPN_ANDROID)
  return new LinuxPingSender(source, parent);
#elif defined(MVPN_MACOS) || defined(MVPN_IOS)
  return new MacOSPingSender(source, parent);
#elif defined(MVPN_WINDOWS)
  return new WindowsPingSender(source, parent);
#else
  return new DummyPingSender(source, parent);
#endif
}
