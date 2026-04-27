/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import AppIntents
import NetworkExtension
import SwiftUI
import WidgetKit
import os

@available(iOS 18.0, *)
struct ToggleWidgetControl: ControlWidget {
  static let kind: String = "org.mozilla.ios.FirefoxVPN.ToggleWidget"

  var body: some ControlWidgetConfiguration {
    StaticControlConfiguration(
      kind: Self.kind,
      provider: Provider()
    ) { value in
      ControlWidgetToggle(
        "Mozilla VPN",
        isOn: value,
        action: ToggleIntent()
      ) { isOn in
        Label(isOn ? "On" : "Off", systemImage: isOn ? "shield.lefthalf.filled" : "shield.lefthalf.filled.slash")
      }
    }
    .displayName("Mozilla VPN") // OMG LOCALIZE
    .description("VPN BUT NEED TO UPDATE THIS") // OMG LOCALIZE
  }
}

@available(iOS 18.0, *)
extension ToggleWidgetControl {

  struct Value {
    var isTurningOn: Bool
  }

  struct Provider: ControlValueProvider {
    var previewValue: Bool { true }

    func currentValue() async -> Bool {
      let logger = Logger(subsystem: "org.mozilla.ios.FirefoxVPN", category: "WidgetControl")
      var tunnel: NETunnelProviderManager?
      do {
        let managers: [NETunnelProviderManager]? = try await withCheckedThrowingContinuation { continuation in
          NETunnelProviderManager.loadAllFromPreferences { managers, error in
            if let error = error {
              logger.warning("Error loading tunnels: \(error.localizedDescription)")
              continuation.resume(throwing: error)
            } else {
              continuation.resume(returning: managers)
            }
          }
        }

        tunnel = (managers?.first(where: {
          guard
            let proto = $0.protocolConfiguration,
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

          logger.debug("Found the manager with the correct bundle identifier: \(bundleIdentifier)")
          return true
        }))


      } catch let error {
        logger.debug("Error: \(error.localizedDescription)")
        return false
      }

      if (tunnel == nil) {
        logger.warning("Creating the tunnel, as none were found")
        tunnel = NETunnelProviderManager()
      }

      let isOn = (tunnel?.connection as? NETunnelProviderSession)?.status == .connected || (tunnel?.connection as? NETunnelProviderSession)?.status == .connecting
      return isOn
    }
  }
}

