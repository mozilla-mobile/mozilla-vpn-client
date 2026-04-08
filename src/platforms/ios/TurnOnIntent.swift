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

  enum Result {
    case success (entryCity: String?, exitCity: String?), errorNoSession, errorAlreadyActive
  }

  @MainActor
  func perform() async throws -> some IntentResult & ProvidesDialog {
    let dialog: IntentDialog
    let activationResult = IOSControllerImpl.startTunnelFromIntent()
    let responseText: LocalizedStringResource
    let responseImage: String

    switch activationResult {
      case .success(let entryCity, let exitCity):
        if let exitCity = exitCity,!exitCity.isEmpty {
          if let entryCity = entryCity, !entryCity.isEmpty {
            responseText = LocalizedStringResource("Mozilla VPN connected through \(exitCity) via \(entryCity)")
          } else {
            responseText = LocalizedStringResource("Mozilla VPN connected through \(exitCity)")
          }
        } else {
          responseText = LocalizedStringResource("Mozilla VPN connected")
        }
        responseImage = TurnOnIntent.systemImageName
      case .errorNoSession:
        responseText = LocalizedStringResource("Error turning on Mozilla VPN")
        responseImage = "exclamationmark.triangle"
      case .errorAlreadyActive:
        responseText = LocalizedStringResource("VPN is already connected")
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
