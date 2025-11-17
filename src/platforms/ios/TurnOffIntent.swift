/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import AppIntents

@available(iOS 16.0, *)
struct TurnOffIntent: AppIntent {
  static let title: LocalizedStringResource = "Turn off Mozilla VPN"

  static let systemImageName = "shield.lefthalf.filled.slash"

  static var parameterSummary: some ParameterSummary {
    Summary("Turn off Mozilla VPN")
  }

  @MainActor
  func perform() async throws -> some IntentResult & ProvidesDialog {
    IOSControllerImpl.stopTunnelFromIntent()
    let responseText = LocalizedStringResource("Mozilla VPN turned off")
    let dialog: IntentDialog
    if #available(iOS 17.2, *) {
      dialog = IntentDialog(full: responseText,
                            systemImageName: TurnOffIntent.systemImageName)
    } else {
      dialog = IntentDialog(responseText)
    }
    return .result(dialog: dialog)
  }
}
