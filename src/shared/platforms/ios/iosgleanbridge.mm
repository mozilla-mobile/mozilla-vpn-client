/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "iosgleanbridge.h"
#include "Mozilla-Swift.h"
#include "feature.h"
#include "leakdetector.h"

#include <QString>

namespace {

// Our Swift singleton.
IOSGleanBridgeImpl* impl = nullptr;

}  // namespace

IOSGleanBridge::IOSGleanBridge(bool isTelemetryEnabled, const QString& channel) {
  MZ_COUNT_CTOR(IOSGleanBridge);

  impl = [[IOSGleanBridgeImpl alloc]
      initWithTelemetryEnabled:isTelemetryEnabled
                       channel:channel.toNSString()
         isGleanDebugTagActive:Feature::get(Feature::Feature_gleanDebugViewTag)->isSupported()];
  Q_ASSERT(impl);
}

IOSGleanBridge::~IOSGleanBridge() {
  MZ_COUNT_DTOR(IOSGleanBridge);

  if (impl) {
    [impl dealloc];
    impl = nullptr;
  }
}

// static
void IOSGleanBridge::syncTelemetryEnabled(bool isTelemetryEnabled) {
  [impl syncTelemetryEnabledWithTelemetryEnabled:isTelemetryEnabled];
}
