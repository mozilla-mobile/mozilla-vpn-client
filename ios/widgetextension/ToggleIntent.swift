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

  static var authenticationPolicy: IntentAuthenticationPolicy = .requiresAuthentication

  static let systemImageName = "bolt.shield"

  @Parameter(title: "VPN is connected")
  var value: Bool

  @MainActor
  func perform() async throws -> some IntentResult & ProvidesDialog {
    let logger = Logger(subsystem: "org.mozilla.ios.FirefoxVPN", category: "ToggleIntent")
    let managers: [NETunnelProviderManager]? = try await withCheckedThrowingContinuation { continuation in
      NETunnelProviderManager.loadAllFromPreferences { managers, error in
        if let error = error {
          logger.warning("Error loading tunnel managers: \(error.localizedDescription)")
          continuation.resume(throwing: error)
        } else {
          continuation.resume(returning: managers)
        }
      }
    }

    let dialog: IntentDialog
    let responseText: LocalizedStringResource
    let responseImage: String
    let tunnel: NETunnelProviderManager
    if let potentialTunnel = (managers?.first(where: { $0.isOurManager })) {
      tunnel = potentialTunnel
    } else {
      logger.warning("Creating the tunnel, as none were found")
      tunnel = NETunnelProviderManager()
    }

    guard let connection = tunnel.connection as? NETunnelProviderSession else {
      logger.error("Tunnel connection is not of the correct type")
      responseText = LocalizedStringResource("vpn.iosAppIntentsMain.toggleError", defaultValue: "Error changing VPN status")
      if #available(iOS 17.2, *) {
        dialog = IntentDialog(full: responseText,
                              supporting: responseText,
                              systemImageName: "exclamationmark.triangle")
      } else {
        dialog = IntentDialog(responseText)
      }
      return .result(dialog: dialog)
    }

    let currentStatus = (connection.status == .connected || connection.status == .connecting)
    if currentStatus == value {
      if !value {
        responseText = LocalizedStringResource("vpn.iosAppIntentsMain.turnOffError", defaultValue: "No active VPN connection")
        responseImage = "exclamationmark.triangle"
      } else {
        responseText = LocalizedStringResource("vpn.iosAppIntentsMain.turnOnAlreadyConnectedError", defaultValue: "VPN is already connected")
        responseImage = "exclamationmark.triangle"
      }
      logger.error("Widget status (\(value)) doesn't match current tunnel status (\(currentStatus)).")
      value = currentStatus
    } else {
      if !value {
        logger.info("Turning off VPN")
        // Turn off auto-connect, otherwise it will immediately reconnect.
        tunnel.isOnDemandEnabled = false;
        tunnel.onDemandRules = []

        try await withCheckedThrowingContinuation { (continuation: CheckedContinuation<Void, Error>) in
          tunnel.saveToPreferences { error in
            if let error = error {
              logger.info("Error saving tunnel: \(error.localizedDescription)")
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
        logger.info("Turning on VPN")
        do {
          // Create a rule so that the VPN always connects. This allows reconnection if
          // the device reboots or the network extension is stopped for an unexpected reason.
          let alwaysConnect = NEOnDemandRuleConnect()
          alwaysConnect.interfaceTypeMatch = .any
          tunnel.isOnDemandEnabled = true
          tunnel.onDemandRules = [alwaysConnect]

          try connection.startTunnel(options: ["source": "control"])
          let config = (tunnel.protocolConfiguration as? NETunnelProviderProtocol)?.providerConfiguration
          let entryCity: String? = config?["entryCity"] as? String
          let exitCity: String? = config?["exitCity"] as? String

          if let exitCity = exitCity, !exitCity.isEmpty {
            if let entryCity = entryCity, !entryCity.isEmpty {
              responseText = LocalizedStringResource("vpn.iosAppIntentsMain.turnOnConfirmationMultiHop", defaultValue: "Mozilla VPN connected through \(exitCity) via \(entryCity)")
            } else {
              responseText = LocalizedStringResource("vpn.iosAppIntentsMain.turnOnConfirmationSingleHop", defaultValue: "Mozilla VPN connected through \(exitCity)")
            }
          } else {
            responseText = LocalizedStringResource("vpn.iosAppIntentsMain.turnOnConfirmation", defaultValue: "Mozilla VPN connected")
          }
          responseImage = "shield.lefthalf.filled"
          value = false
        } catch let error {
          logger.warning("Error: \(error.localizedDescription)")
          responseText = LocalizedStringResource("vpn.iosAppIntentsMain.turnOnError", defaultValue: "Error turning on Mozilla VPN")
          responseImage = "exclamationmark.triangle"
        }
      }
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
