/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import IOSGlean

public class IOSGleanBridgeImpl : NSObject {
    private static let logger = IOSLoggerImpl(tag: "IOSGleanBridge")

  @objc init(telemetryEnabled isTelemetryEnabled: Bool, channel appChannel: String, isGleanDebugTagActive: Bool) {
      super.init()
      
      Glean.shared.registerPings(GleanMetrics.Pings.self)
      if isGleanDebugTagActive {
        IOSGleanBridgeImpl.logger.info(message: "Setting Glean debug tag.")
        Glean.shared.setDebugViewTag("VPNTest")
      }
      Glean.shared.initialize(
          uploadEnabled: isTelemetryEnabled,
          configuration: Configuration.init(channel: appChannel),
          buildInfo: GleanMetrics.GleanBuild.info
      )
      
      withDefaults { defaults in
          defaults.set(isTelemetryEnabled, forKey: Constants.UserDefaultKeys.telemetryEnabled)
          defaults.set(appChannel, forKey: Constants.UserDefaultKeys.appChannel)
          defaults.synchronize()
      }
  }

  @objc func syncTelemetryEnabled(telemetryEnabled isTelemetryEnabled: Bool) {
      withDefaults { defaults in
          defaults.set(isTelemetryEnabled, forKey: Constants.UserDefaultKeys.telemetryEnabled)
      }
      
      if let session = TunnelManager.session {
        do {
            IOSGleanBridgeImpl.logger.info(message: "Attempting to send telemetry state change message.");
          try session.sendProviderMessage(
              TunnelMessage.telemetryEnabledChanged(isTelemetryEnabled).encode()
          ) { _ in
              IOSGleanBridgeImpl.logger.info(message: "Telemetry state change message sent.")
          }
        } catch {
            IOSGleanBridgeImpl.logger.error(message: "Error sending telemetry state change message: \(error)");
        }
      }
  }

    private func withDefaults(_ f: (_ defaults: UserDefaults) -> Void) {
        // This logs an error like so:
        // "[User Defaults] Couldn't read values in CFPrefsPlistSource<0x2821ced00>..."
        // This is just a warning and can be ignored.
        let defaults = UserDefaults(suiteName: Constants.appGroupIdentifier)
        if (defaults == nil) {
            IOSGleanBridgeImpl.logger.error(message: "Attempted to access UserDefaults, but it's not available.")
            return
        }
        
        f(defaults!)
    }
}
