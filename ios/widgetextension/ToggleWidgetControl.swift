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
        LocalizedStringResource("vpn.iosAppIntentsMain.toggleTitle", defaultValue: "Toggle Mozilla VPN"),
        isOn: value,
        action: ToggleIntent()
      ) { isOn in
        Label(isOn ? "On" : "Off", systemImage: isOn ? "shield.lefthalf.filled" : "shield.lefthalf.filled.slash")
      }
    }
    .displayName("Mozilla VPN") // Not localizing - this is localized in another part of the app (that we can't easily get to Apple-land), and seems like all locales translate it as "Mozilla VPN"
    .description(LocalizedStringResource("vpn.iosAppIntentsMain.toggleDescription", defaultValue: "Changes Mozilla VPN status"))
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
      let logger = IOSLoggerImpl(tag: "WidgetControl")
      do {

        guard let tunnel = try await NETunnelProviderManager.ourVpnTunnel() else {
          logger.info(message: "No tunnel found")
          return false
        }
        return tunnel.isConnected

      } catch let error {
        logger.debug(message: "Error: \(error.localizedDescription)")
        return false
      }
    }
  }
}

