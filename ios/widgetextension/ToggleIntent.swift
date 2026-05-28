/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import AppIntents
import NetworkExtension
import os
import WidgetKit

@available(iOS 16.0, *)
struct ToggleIntent: SetValueIntent {
  static let title = LocalizedStringResource("vpn.iosAppIntentsMain.toggleTitle", defaultValue: "Toggle Mozilla VPN")
  static let description = IntentDescription(LocalizedStringResource("vpn.iosAppIntentsMain.toggleDescription", defaultValue: "Changes Mozilla VPN status"))

  static var isDiscoverable = false

  static var authenticationPolicy: IntentAuthenticationPolicy = .requiresAuthentication
  static let errorSystemImageName = "exclamationmark.triangle"

  @Parameter(title: "VPN is connected")
  var value: Bool

  init() {}

  init(value: Bool) {
    self.value = value
  }

  @MainActor
  func perform() async throws -> some IntentResult & ProvidesDialog {
    let logger = IOSLoggerImpl(tag: "ToggleIntent")
    let dialog: IntentDialog
    let responseText: LocalizedStringResource
    let responseImage: String

    let tunnel: NETunnelProviderManager
    if let foundTunnel = try await NETunnelProviderManager.ourVpnTunnel() {
      tunnel = foundTunnel
    } else {
      logger.error(message: "Creating tunnel, as none was found")
      tunnel = NETunnelProviderManager()
    }

    guard let connection = tunnel.connection as? NETunnelProviderSession else {
      logger.error(message: "Tunnel connection is not of the correct type")
      responseText = LocalizedStringResource("vpn.iosAppIntentsMain.toggleError", defaultValue: "Error changing VPN status")
      if #available(iOS 17.2, *) {
        dialog = IntentDialog(full: responseText,
                              supporting: responseText,
                              systemImageName: ToggleIntent.errorSystemImageName)
      } else {
        dialog = IntentDialog(responseText)
      }
      return .result(dialog: dialog)
    }

    let currentStatus = tunnel.isConnected
    if currentStatus == value {
      if !value {
        responseText = LocalizedStringResource("vpn.iosAppIntentsMain.turnOffError", defaultValue: "No active VPN connection")
        responseImage = ToggleIntent.errorSystemImageName
      } else {
        responseText = LocalizedStringResource("vpn.iosAppIntentsMain.turnOnAlreadyConnectedError", defaultValue: "VPN is already connected")
        responseImage = ToggleIntent.errorSystemImageName
      }
      logger.error(message: "Widget status (\(value)) doesn't match current tunnel status (\(currentStatus)).")
      value = currentStatus
    } else {
      if !value {
        logger.info(message: "Turning off VPN")
        // Turn off auto-connect, otherwise it will immediately reconnect.
        tunnel.isOnDemandEnabled = false;
        tunnel.onDemandRules = []

        try await withCheckedThrowingContinuation { (continuation: CheckedContinuation<Void, Error>) in
          tunnel.saveToPreferences { error in
            if let error = error {
              logger.info(message: "Error saving tunnel: \(error.localizedDescription)")
              continuation.resume(throwing: error)
            } else {
              continuation.resume()
            }
          }
        }

        connection.stopTunnel()
        responseText = LocalizedStringResource("vpn.iosAppIntentsMain.turnOffConfirmation", defaultValue: "Mozilla VPN disconnected")
        responseImage = "shield.lefthalf.filled.slash"
        value = true
      } else {
        logger.info(message: "Turning on VPN")
        do {
          // Create a rule so that the VPN always connects. This allows reconnection if
          // the device reboots or the network extension is stopped for an unexpected reason.
          let alwaysConnect = NEOnDemandRuleConnect()
          alwaysConnect.interfaceTypeMatch = .any
          tunnel.isOnDemandEnabled = true
          tunnel.onDemandRules = [alwaysConnect]

          try await withCheckedThrowingContinuation { (continuation: CheckedContinuation<Void, Error>) in
            tunnel.saveToPreferences { error in
              if let error = error {
                logger.info(message: "Error saving tunnel: \(error.localizedDescription)")
                continuation.resume(throwing: error)
              } else {
                continuation.resume()
              }
            }
          }

          try connection.startTunnel(options: ["source": "control"])
          responseText = tunnel.turnOnConfirmation
          responseImage = "shield.lefthalf.filled"
          value = false
        } catch let error {
          logger.error(message: "Error: \(error.localizedDescription)")
          responseText = LocalizedStringResource("vpn.iosAppIntentsMain.turnOnError", defaultValue: "Error turning on Mozilla VPN")
          responseImage = ToggleIntent.errorSystemImageName
        }
      }
    }

    // We should not need this next block, as it should be handled in
    // iostunnel.swift. However, closing the tunnel seems to kill the
    // network extension quickly, and when activated/deactivate from
    // control center toggle, we weren't always updating any widgets,
    // and vice versa. This seemed to fix it.
    // However, the turn on and turn off intents work just fine for
    // both widgets and controls, it seems, so they do not need this.
    WidgetCenter.shared.reloadAllTimelines()
    if #available(iOS 18.0, *) {
        ControlCenter.shared.reloadAllControls()
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
