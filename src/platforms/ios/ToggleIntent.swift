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

  let logger = Logger(subsystem: "org.mozilla.BLAH", category: "api")

  @MainActor
  func perform() async throws -> some IntentResult & ProvidesDialog {
//    value = !value
//    ControlCenter.shared.reloadAllControls()
//    return .result(dialog: IntentDialog("HAHA"))



    logger.info("MATTHEW UP")
    let managers: [NETunnelProviderManager]? = try await withCheckedThrowingContinuation { continuation in
             NETunnelProviderManager.loadAllFromPreferences { managers, error in
                 if let error = error {
                   logger.info("MATTHEW ERROR \(error.localizedDescription)")
                     continuation.resume(throwing: error)
                 } else {
                   logger.info("MATTHEW GOOD")
                     continuation.resume(returning: managers)
                 }
             }
         }
    logger.info("MATTHEW \(managers?.count ?? -1)")
//    return .result(dialog: IntentDialog("hey"))
//    NETunnelProviderManager.loadAllFromPreferences { managers, error in
      let dialog: IntentDialog
      let responseText: LocalizedStringResource
      let responseImage: String



      var tunnel: NETunnelProviderManager?
      //        guard let self = self else {
      //            return
      //        }

//      if let error = error {
//        //            TunnelManager.logger.error(message: "Loading from preference failed: \(error)")
//        return
//      }

      //        let nsManagers = managers ?? []
      //        TunnelManager.logger.debug(message: "We have received \(nsManagers.count) managers.")

      tunnel = (managers?.first(where: {
        guard
          let proto = $0.protocolConfiguration,
          let tunnelProto = proto as? NETunnelProviderProtocol
        else {
          logger.debug("Ignoring manager because the proto is invalid.")
          return false
        }

        guard let bundleIdentifier = tunnelProto.providerBundleIdentifier else {
          logger.debug("Ignoring manager because the bundle identifier is null.")
          return false
        }

        logger.debug("MATTHEW: \(bundleIdentifier)")
        if (bundleIdentifier != "org.mozilla.ios.FirefoxVPN.network-extension") {
          logger.debug("Ignoring manager because the bundle identifier doesn't match.")
          return false;
        }

        logger.debug("Found the manager with the correct bundle identifier: \(bundleIdentifier)")
        return true
      }))



//    logger.info("MATTHEW \(tunnel.debugDescription)")
      if (tunnel == nil) {
        //            TunnelManager.logger.debug(message: "Creating the tunnel")
        tunnel = NETunnelProviderManager()
      }

      let currentStatus = (tunnel?.connection as? NETunnelProviderSession)?.status == .connected || (tunnel?.connection as? NETunnelProviderSession)?.status == .connecting
    if false { //} currentStatus != value {
        logger.info("MATTHEW EEEEK \(currentStatus) and \(value)")
        if !value {
          responseText = LocalizedStringResource("vpn.iosAppIntentsMain.turnOffError", defaultValue: "No active VPN connection")
          responseImage = "exclamationmark.triangle"
        } else {
          responseText = LocalizedStringResource("vpn.iosAppIntentsMain.turnOnAlreadyConnectedError", defaultValue: "VPN is already connected")
          responseImage = "exclamationmark.triangle"
        }
        value = currentStatus
        logger.info("MATTHEW EEEEK updated \(value)")
      } else {
        logger.info("MATTHEW UP TOP")

        if value {
          // IF DOING IT THIS WAY, MUST REMEMBER TO REMOVE THE AUTO RECONNECT STUFF
          // Turn off auto-connect, otherwise it will immediately reconnect.
          tunnel?.isOnDemandEnabled = false;
          tunnel?.onDemandRules = []

          try await withCheckedThrowingContinuation { (continuation: CheckedContinuation<Void, Error>) in
                    tunnel?.saveToPreferences { error in
                      if let error = error {
                        logger.info("MATTHEW ERROR 2 \(error.localizedDescription)")
                          continuation.resume(throwing: error)
                      } else {
                        logger.info("MATTHEW GOOD 2")
                          continuation.resume()
                      }
                    }
           }



          let wasSuccessfullyDeactivated = true
          let tempStuff = tunnel?.connection as? NETunnelProviderSession
          logger.info("MATTHEW NEW WORK \(tempStuff.debugDescription)")
          (tunnel?.connection as? NETunnelProviderSession)?.stopTunnel()

          //        let wasSuccessfullyDeactivated = IOSControllerImpl.stopTunnelFromIntent()
          if wasSuccessfullyDeactivated {
            responseText = LocalizedStringResource("vpn.iosAppIntentsMain.turnOffConfirmation", defaultValue: "Mozilla VPN disconnected")
            responseImage = "shield.lefthalf.filled.slash"
            value = true
          } else {
            responseText = LocalizedStringResource("vpn.iosAppIntentsMain.turnOffError", defaultValue: "No active VPN connection")
            responseImage = "exclamationmark.triangle"
          }
        } else {
          logger.info("MATTHEW IN TURN ON")
          let activationResult: Bool
          do {
            // IF DOING IT THIS WAY, MUST REMEMBER TO ADD THE AUTO RECONNECT STUFF
            logger.info("MATTHEW GOOD 3.0")
            try (tunnel?.connection as? NETunnelProviderSession)?.startTunnel(options: ["source": "control"])
            logger.info("MATTHEW GOOD 3.1")
            activationResult = true
          } catch let error {
            logger.info("MATTHEW ERROR 3 \(error.localizedDescription)")
            activationResult = false
          }
          logger.info("MATTHEW OUT")
          if activationResult {
            // Fix 1: use TunnelManager.protocolConfiguration instead of NETunnelProviderManager()
            let config = (tunnel?.protocolConfiguration as? NETunnelProviderProtocol)?.providerConfiguration
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
          } else {
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
//    try await Task.sleep(for: .seconds(4))
      return .result(dialog: dialog)
    }
//  }
}
