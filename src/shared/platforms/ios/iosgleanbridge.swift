/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import IOSGlean

public class IOSGleanBridgeImpl : NSObject {
  @objc init(telemetryEnabled isTelemetryEnabled: Bool, channel appChannel: String) {
    Glean.shared.registerPings(GleanMetrics.Pings.self)
    Glean.shared.initialize(
      uploadEnabled: isTelemetryEnabled,
      configuration: Configuration.init(channel: appChannel),
      buildInfo: GleanMetrics.GleanBuild.info
      )
  }
}
