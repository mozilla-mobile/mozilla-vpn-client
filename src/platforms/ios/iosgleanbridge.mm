/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "iosgleanbridge.h"
#include "leakdetector.h"
#include "Mozilla_VPN-Swift.h"

namespace {

// Our Swift singleton.
IOSGleanBridgeImpl* impl = nullptr;

}  // namespace

IOSGleanBridge::IOSGleanBridge(bool isTelemetryEnabled, NSString* channel) {
  MVPN_COUNT_CTOR(IOSGleanBridge);

  impl = [[IOSGleanBridgeImpl alloc] initWithTelemetryEnabled:isTelemetryEnabled channel:channel];
  Q_ASSERT(impl);
}

IOSGleanBridge::~IOSGleanBridge() {
  MVPN_COUNT_DTOR(IOSGleanBridge);

  if (impl) {
    [impl dealloc];
    impl = nullptr;
  }
}
