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
      let managers: [NETunnelProviderManager]?
      do {
        managers = try await withCheckedThrowingContinuation { continuation in
          NETunnelProviderManager.loadAllFromPreferences { managers, error in
            if let error = error {
              logger.warning("Error loading tunnels: \(error.localizedDescription)")
              continuation.resume(throwing: error)
            } else {
              continuation.resume(returning: managers)
            }
          }
        }
      } catch let error {
        logger.debug("Error: \(error.localizedDescription)")
        return false
      }

      guard let managers = managers, let tunnel = (managers.first(where: { $0.isOurManager })) else {
        logger.warning("No tunnel found")
        return false
      }

      return tunnel.isConnected
    }
  }
}

