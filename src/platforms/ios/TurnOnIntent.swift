/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import AppIntents

@available(iOS 16.0, *)
struct TurnOnIntent: AppIntent {
  static let title: LocalizedStringResource = "Turn On Mozilla VPN"

  static let description: LocalizedStringResource = "Activate the Mozilla VPN connection"

  static var authenticationPolicy: IntentAuthenticationPolicy = .requiresAuthentication

  static let systemImageName = "shield.lefthalf.filled"

  @MainActor
  func perform() async throws -> some IntentResult & ProvidesDialog {
    let dialog: IntentDialog
    let wasSuccessfullyActivated = IOSControllerImpl.startTunnelFromIntent()
    let responseText: LocalizedStringResource
    let responseImage: String
    if wasSuccessfullyActivated {
      responseText = LocalizedStringResource("Mozilla VPN turned on")
      responseImage = TurnOnIntent.systemImageName
    } else {
      responseText = LocalizedStringResource("Error turning on Mozilla VPN")
      responseImage = "exclamationmark.triangle"
    }
    if #available(iOS 17.2, *) {
      dialog = IntentDialog(full: responseText,
                            systemImageName: responseImage)
    } else {
      dialog = IntentDialog(responseText)
    }
    return .result(dialog: dialog)
  }
}
