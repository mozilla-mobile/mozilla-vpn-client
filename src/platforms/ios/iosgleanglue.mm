/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "iosgleanglue.h"
#include "Mozilla_VPN-Swift.h"

namespace {

// Our Swift singleton.
IOSGleanGlueImpl* impl = nullptr;

}  // namespace

IOSGleanGlue::IOSGleanGlue(bool isTelemetryEnabled) {
//  MVPN_COUNT_CTOR(IOSGleanGlue);
  
  impl = [[IOSGleanGlueImpl alloc] initWithTelemetryEnabled:isTelemetryEnabled];

  Q_ASSERT(!impl);
}

IOSGleanGlue::~IOSGleanGlue() {
//  MVPN_COUNT_DTOR(IOSGleanGlue);

  if (impl) {
    [impl dealloc];
    impl = nullptr;
  }
}
