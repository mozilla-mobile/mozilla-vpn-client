/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Glean

public class IOSGleanGlueImpl : NSObject {

  @objc init(telemetryEnabled isTelemetryEnabled: Bool) {
      // If you have custom pings in your application, you must register them
      // using the following command. This command should be omitted for
      // applications not using custom pings.
      Glean.shared.registerPings(GleanMetrics.Pings.self)

      // Initialize the Glean library.
      Glean.shared.initialize(
          // Here, `Settings` is a method to get user preferences specific to
          // your application, and not part of the Glean API.
        uploadEnabled: isTelemetryEnabled,
        buildInfo: GleanMetrics.GleanBuild.info
      )
  }

}
