/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import NetworkExtension
import os

extension NETunnelProviderManager {
  static func ourVpnTunnel() async throws -> NETunnelProviderManager? {
    let managers = try await NETunnelProviderManager.loadAllFromPreferences()
    return (managers.first(where: { $0.isOurManager }))
  }

  var isOurManager: Bool {
      let logger = IOSLoggerImpl(tag: "NETunnelProviderManager")
      guard
          let proto = self.protocolConfiguration,
          let tunnelProto = proto as? NETunnelProviderProtocol
      else {
          logger.debug(message: "Ignoring manager because the proto is invalid.")
          return false
      }

      guard let bundleIdentifier = tunnelProto.providerBundleIdentifier else {
          logger.debug(message: "Ignoring manager because the bundle identifier is null.")
          return false
      }

      if (bundleIdentifier != "org.mozilla.ios.FirefoxVPN.network-extension") {
          logger.debug(message: "Ignoring manager because the bundle identifier doesn't match.")
          return false;
      }

      return true
  }

  var isConnected: Bool {
    guard let connection = self.connection as? NETunnelProviderSession else {
        let logger = IOSLoggerImpl(tag: "NETunnelProviderManager")
        logger.error(message: "Tunnel connection not proper type")
        return false
    }

    let isOn = (connection.status == .connected || connection.status == .connecting)
    return isOn
  }

  var turnOnConfirmation: LocalizedStringResource {
    guard let config = (self.protocolConfiguration as? NETunnelProviderProtocol)?.providerConfiguration,
          let exitCity = config["exitCity"] as? String, !exitCity.isEmpty else {
      let logger = IOSLoggerImpl(tag: "NETunnelProviderManager")
      logger.error(message: "Did not find a city")
      return LocalizedStringResource("vpn.iosAppIntentsMain.turnOnConfirmation", defaultValue: "Mozilla VPN connected")
    }

    if let entryCity = config["entryCity"] as? String, !entryCity.isEmpty {
      return LocalizedStringResource("vpn.iosAppIntentsMain.turnOnConfirmationMultiHop", defaultValue: "Mozilla VPN connected through \(exitCity) via \(entryCity)")
    } else {
      return LocalizedStringResource("vpn.iosAppIntentsMain.turnOnConfirmationSingleHop", defaultValue: "Mozilla VPN connected through \(exitCity)")
    }
  }
}
