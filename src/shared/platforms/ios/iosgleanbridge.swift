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
<<<<<<< HEAD
=======

  @objc func syncTelemetryEnabled(telemetryEnabled isTelemetryEnabled: Bool) {
      withDefaults { defaults in
          defaults.set(isTelemetryEnabled, forKey: Constants.UserDefaultKeys.telemetryEnabled)
      }
      
      if let session = TunnelManager.session {
        do {
            Logger.global?.log(message: "Attempting to send telemetry state change message.");
          try session.sendProviderMessage(
              TunnelMessage.telemetryEnabledChanged(isTelemetryEnabled).encode()
          ) { _ in
              Logger.global?.log(message: "Telemetry state change message sent.")
          }
        } catch {
            Logger.global?.log(message: "Error sending telemetry state change message: \(error)");
        }
      }
  }

    private func withDefaults(_ f: (_ defaults: UserDefaults) -> Void) {
        // This logs an error like so:
        // "[User Defaults] Couldn't read values in CFPrefsPlistSource<0x2821ced00>..."
        // This is just a warning and can be ignored.
        let defaults = UserDefaults(suiteName: Constants.appGroupIdentifier)
        if (defaults == nil) {
            Logger.global?.log(message: "Attempted to access UserDefaults, but it's not available.")
            return
        }
        
        f(defaults!)
    }
>>>>>>> 6b1373492 (VPN-5035 - Always execute TunnelManager.initialize completion handler (#7256) (#7281))
}
