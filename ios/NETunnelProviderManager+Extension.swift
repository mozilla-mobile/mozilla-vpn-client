/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import NetworkExtension
import os

extension NETunnelProviderManager {
  var isOurManager: Bool {
    let logger = Logger(subsystem: "NETunnelProviderManager", category: "MozillaVPN")
      guard
          let proto = self.protocolConfiguration,
          let tunnelProto = proto as? NETunnelProviderProtocol
      else {
          logger.debug("Ignoring manager because the proto is invalid.")
          return false
      }

      guard let bundleIdentifier = tunnelProto.providerBundleIdentifier else {
          logger.debug("Ignoring manager because the bundle identifier is null.")
          return false
      }

      if (bundleIdentifier != "org.mozilla.ios.FirefoxVPN.network-extension") {
          logger.debug("Ignoring manager because the bundle identifier doesn't match.")
          return false;
      }

      logger.debug("Found the manager with the correct bundle identifier")
      return true
  }

  var isConnected: Bool {
    let logger = Logger(subsystem: "NETunnelProviderManager", category: "MozillaVPN")
    guard let connection = self.connection as? NETunnelProviderSession else {
        logger.error("Tunnel connection not proper type")
        return false
    }

    let isOn = (connection.status == .connected || connection.status == .connecting)
    return isOn
  }
}
