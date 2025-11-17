/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import AppIntents

@available(iOS 16.0, *)
struct TurnOnIntent: AppIntent {
  static let title: LocalizedStringResource = "Turn on Mozilla VPN"

  static let systemImageName = "shield.lefthalf.filled"

  static var parameterSummary: some ParameterSummary {
    Summary("Turn on Mozilla VPN")
  }

  @MainActor
  func perform() async throws -> some IntentResult & ProvidesDialog {
    IOSControllerImpl.startTunnelFromIntent()
    let responseText = LocalizedStringResource("Mozilla VPN turned on")
    let dialog: IntentDialog
    if #available(iOS 17.2, *) {
      dialog = IntentDialog(full: responseText,
                            systemImageName: TurnOnIntent.systemImageName)
    } else {
      dialog = IntentDialog(responseText)
    }
    return .result(dialog: dialog)
  }
}
