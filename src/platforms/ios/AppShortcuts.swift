/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import AppIntents

@available(iOS 16.0, *)
struct MozillaVPNAppShortcuts: AppShortcutsProvider {

  static var shortcutTileColor = ShortcutTileColor.purple

  // There are no phrases for ToggleIntent here, which is intentional. We do not use Siri phrases to
  // activate it; it is only used to control widgets/controls.

  static var appShortcuts: [AppShortcut] {
    AppShortcut(
      intent: TurnOnIntent(),
      phrases: [
        "Activate \(.applicationName)",
        "Turn on \(.applicationName)",
        "Turn \(.applicationName) on"
      ],
      shortTitle: LocalizedStringResource("vpn.iosAppIntentsMain.turnOnAction", defaultValue: "Turn On Mozilla VPN"),
      systemImageName: "shield.lefthalf.filled"
    )
    AppShortcut(
      intent: TurnOffIntent(),
      phrases: [
        "Deactivate \(.applicationName)",
        "Turn off \(.applicationName)",
        "Turn \(.applicationName) off"
      ],
      shortTitle: LocalizedStringResource("vpn.iosAppIntentsMain.turnOffAction", defaultValue: "Turn Off Mozilla VPN"),
      systemImageName: "shield.lefthalf.filled.slash"
    )
    AppShortcut(
      intent: VPNStatusIntent(),
      phrases: [
        "Is \(.applicationName) currently active?",
        "Is \(.applicationName) currently activated?",
        "Is \(.applicationName) currently turned on?",
        "Is \(.applicationName) currently switched on?",
        "Is \(.applicationName) currently connected?",
        "Is \(.applicationName) active?",
        "Is \(.applicationName) activated?",
        "Is \(.applicationName) turned on?",
        "Is \(.applicationName) switched on?",
        "Is \(.applicationName) connected?"
      ],
      shortTitle: LocalizedStringResource("vpn.iosAppIntentsMain.statusQueryTitle", defaultValue: "Is the VPN currently connected?"),
      systemImageName: "questionmark.square.dashed"
    )
  }
}
