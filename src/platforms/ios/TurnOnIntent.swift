/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import AppIntents

@available(iOS 16.0, *)
struct TurnOnIntent: AppIntent {
  static let title = LocalizedStringResource("vpn.iosAppIntentsMain.turnOnAction", defaultValue: "Turn On Mozilla VPN")

  static let description = IntentDescription(LocalizedStringResource("vpn.iosAppIntentsMain.turnOnDescription", defaultValue: "Activate the Mozilla VPN connection"))

  static var authenticationPolicy: IntentAuthenticationPolicy = .requiresAuthentication

  static let systemImageName = "shield.lefthalf.filled"

  enum Result {
    case success(turnOnConfirmation: LocalizedStringResource), errorNoSession, errorAlreadyActive
  }

  @MainActor
  func perform() async throws -> some IntentResult & ProvidesDialog {
    let dialog: IntentDialog
    let activationResult = await IOSControllerImpl.startTunnelFromIntent()
    let responseText: LocalizedStringResource
    let responseImage: String

    switch activationResult {
      case .success(let turnOnConfirmation):
        responseText = turnOnConfirmation
        responseImage = TurnOnIntent.systemImageName
      case .errorNoSession:
        responseText = LocalizedStringResource("vpn.iosAppIntentsMain.turnOnError", defaultValue: "Error turning on Mozilla VPN")
        responseImage = "exclamationmark.triangle"
      case .errorAlreadyActive:
        responseText = LocalizedStringResource("vpn.iosAppIntentsMain.turnOnAlreadyConnectedError", defaultValue: "VPN is already connected")
        responseImage = "exclamationmark.triangle"

    }

    if #available(iOS 17.2, *) {
      dialog = IntentDialog(full: responseText,
                            supporting: responseText,
                            systemImageName: responseImage)
    } else {
      dialog = IntentDialog(responseText)
    }
    return .result(dialog: dialog)
  }
}
