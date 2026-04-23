//
//  ToggleWidgetControl.swift
//  ToggleWidget
//
//  Created by Matt Cleinman on 4/17/26.
//

import AppIntents
import NetworkExtension
import SwiftUI
import WidgetKit
import os

struct ToggleWidgetControl: ControlWidget {
    static let kind: String = "org.mozilla.ios.FirefoxVPN.ToggleWidget"
    static let logger = Logger(subsystem: "org.mozilla.BLAH", category: "api")

    var body: some ControlWidgetConfiguration {
        StaticControlConfiguration(
            kind: Self.kind,
            provider: Provider()
        ) { value in
            ControlWidgetToggle(
                "Mozilla VPN",
                isOn: value,
                action: ToggleIntent()
            ) { isOn in
              Label(isOn ? "On" : "Off", systemImage: isOn ? "shield.lefthalf.filled" : "shield.lefthalf.filled.slash")
            }
        }
        .displayName("Mozilla VPN") // OMG LOCALIZE
        .description("VPN BUT NEED TO UPDATE THIS")
    }
}

extension ToggleWidgetControl {

    struct Value {
        var isTurningOn: Bool
    }

  struct Provider: ControlValueProvider {
        var previewValue: Bool { true }
//        func previewValue() -> Value {
//            ToggleWidgetControl.Value(isTurningOn: true)
//        }

        func currentValue() async -> Bool {
          var tunnel: NETunnelProviderManager?
          do {
            let managers: [NETunnelProviderManager]? = try await withCheckedThrowingContinuation { continuation in
              NETunnelProviderManager.loadAllFromPreferences { managers, error in
                if let error = error {
                  logger.info("SETH ERROR \(error.localizedDescription)")
                  continuation.resume(throwing: error)
                } else {
                  logger.info("SETH GOOD")
                  continuation.resume(returning: managers)
                }
              }
            }
//          logger.info("SETH \(managers?.count ?? -1)")
      //    return .result(dialog: IntentDialog("hey"))
      //    NETunnelProviderManager.loadAllFromPreferences { managers, error in




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

              logger.debug("SETH: \(bundleIdentifier)")
              if (bundleIdentifier != "org.mozilla.ios.FirefoxVPN.network-extension") {
                logger.debug("Ignoring manager because the bundle identifier doesn't match.")
                return false;
              }

              logger.debug("Found the manager with the correct bundle identifier: \(bundleIdentifier)")
              return true
            }))


          } catch let error {
            logger.debug("SETH UGH \(error.localizedDescription)")
            return false
          }



//          logger.info("SETH \(tunnel.debugDescription)")
            if (tunnel == nil) {
              //            TunnelManager.logger.debug(message: "Creating the tunnel")
              tunnel = NETunnelProviderManager()
            }

            let isOn = (tunnel?.connection as? NETunnelProviderSession)?.status == .connected || (tunnel?.connection as? NETunnelProviderSession)?.status == .connecting

          let tempStatus = (tunnel?.connection as? NETunnelProviderSession)?.status.rawValue
          logger.info("SETH \(isOn) from \(String(tempStatus ?? -5))")


//            let isOn = (NETunnelProviderManager().connection as? NETunnelProviderSession)?.status == .connected
            return isOn
        }
    }
}

//struct TimerConfiguration: ControlConfigurationIntent {
//    static let title: LocalizedStringResource = "Timer Name Configuration"
//
//    @Parameter(title: "Timer Name", default: "Timer")
//    var timerName: String
//}
