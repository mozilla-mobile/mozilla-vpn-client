/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import AppIntents

@available(iOS 16.0, *)
struct VPNStatusIntent: AppIntent {
  static let title = LocalizedStringResource("vpn.iosAppIntentsMain.statusQueryTitle", defaultValue: "Is the VPN currently connected?")

  static let description = IntentDescription(LocalizedStringResource("vpn.iosAppIntentsMain.statusQueryDescription", defaultValue: "Queries current Mozilla VPN connection status"))

  static var authenticationPolicy: IntentAuthenticationPolicy = .requiresAuthentication

  static let systemImageName = "questionmark.square.dashed"

  @MainActor
  func perform() async throws -> some ReturnsValue<Bool> & ProvidesDialog {
    let dialog: IntentDialog
    let responseText: LocalizedStringResource
    let responseImage: String

    let isCurrentlyConnected = TunnelManager.session?.status == .connected

    if isCurrentlyConnected {
        responseText = LocalizedStringResource("vpn.iosAppIntentsMain.statusQueryResponseConnected", defaultValue: "Mozilla VPN is currently connected")
        responseImage = "shield.lefthalf.filled"
    } else {
        responseText = LocalizedStringResource("vpn.iosAppIntentsMain.statusQueryResponseDisconnected", defaultValue: "Mozilla VPN is currently disconnected")
        responseImage = "shield.lefthalf.filled.slash"
    }

    if #available(iOS 17.2, *) {
      dialog = IntentDialog(full: responseText,
                            supporting: responseText,
                            systemImageName: responseImage)
    } else {
      dialog = IntentDialog(responseText)
    }
    return .result(value: isCurrentlyConnected, dialog: dialog)
  }
}
