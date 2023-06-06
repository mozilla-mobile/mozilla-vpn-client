/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import NetworkExtension
import os
import IOSGlean

class PacketTunnelProvider: NEPacketTunnelProvider {
    private lazy var adapter: WireGuardAdapter = {
        return WireGuardAdapter(with: self) { logLevel, message in
            NSLog(message)
        }
    }()

    override init() {
        super.init()
        
        Logger.configureGlobal(tagged: "NET", withFilePath: FileManager.logFileURL?.path)

        // Copied from https://github.com/mozilla/glean/blob/c501555ad63051a4d5813957c67ae783afef1996/glean-core/ios/Glean/Utils/Utils.swift#L90
        let paths = FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask)
        let documentsDirectory = paths[0]
        // We just can't use "glean_data". Otherwise we will crash with the main Glean storage.
        let gleanDataPath = documentsDirectory.appendingPathComponent("glean_netext_data")
        
        let defaults = UserDefaults(suiteName: Constants.appGroupIdentifier)
        let telemetryEnabled = defaults!.bool(forKey: Constants.UserDefaultKeys.telemetryEnabled)
        let appChannel = defaults!.string(forKey: Constants.UserDefaultKeys.appChannel)
        
        Glean.shared.registerPings(GleanMetrics.Pings.self)
        Glean.shared.initialize(
            uploadEnabled: telemetryEnabled,
            configuration: Configuration.init(
                channel: appChannel != nil ? appChannel : "unknown",
                dataPath: gleanDataPath.relativePath
            ),
            buildInfo: GleanMetrics.GleanBuild.info
        )
        
        GleanMetrics.Pings.shared.daemon.submit()
    }

    deinit {
        // TODO (VPN-XXXX): Add Glean shutdown here
    }

    override func startTunnel(options: [String: NSObject]?, completionHandler: @escaping (Error?) -> Void) {
        let activationAttemptId = options?["activationAttemptId"] as? String
        let errorNotifier = ErrorNotifier(activationAttemptId: activationAttemptId)

        NSLog("Starting tunnel from the " + (activationAttemptId == nil ? "OS directly, rather than the app" : "app"))

        guard let tunnelProviderProtocol = self.protocolConfiguration as? NETunnelProviderProtocol,
              let tunnelConfiguration = tunnelProviderProtocol.asTunnelConfiguration() else {
            errorNotifier.notify(PacketTunnelProviderError.savedProtocolConfigurationIsInvalid)
            completionHandler(PacketTunnelProviderError.savedProtocolConfigurationIsInvalid)
            return
        }

        // Start the tunnel
        adapter.start(tunnelConfiguration: tunnelConfiguration) { adapterError in
            guard let adapterError = adapterError else {
                let interfaceName = self.adapter.interfaceName ?? "unknown"

                NSLog("Tunnel interface is \(interfaceName)")

                completionHandler(nil)
                return
            }

            switch adapterError {
            case .cannotLocateTunnelFileDescriptor:
                wg_log(.error, staticMessage: "Starting tunnel failed: could not determine file descriptor")
                errorNotifier.notify(PacketTunnelProviderError.couldNotDetermineFileDescriptor)
                completionHandler(PacketTunnelProviderError.couldNotDetermineFileDescriptor)

            case .dnsResolution(let dnsErrors):
                let hostnamesWithDnsResolutionFailure = dnsErrors.map { $0.address }
                    .joined(separator: ", ")
                NSLog("DNS resolution failed for the following hostnames: \(hostnamesWithDnsResolutionFailure)")
                errorNotifier.notify(PacketTunnelProviderError.dnsResolutionFailure)
                completionHandler(PacketTunnelProviderError.dnsResolutionFailure)

            case .setNetworkSettings(let error):
                NSLog("Starting tunnel failed with setTunnelNetworkSettings returning \(error.localizedDescription)")
                errorNotifier.notify(PacketTunnelProviderError.couldNotSetNetworkSettings)
                completionHandler(PacketTunnelProviderError.couldNotSetNetworkSettings)

            case .startWireGuardBackend(let errorCode):
                NSLog("Starting tunnel failed with wgTurnOn returning \(errorCode)")
                errorNotifier.notify(PacketTunnelProviderError.couldNotStartBackend)
                completionHandler(PacketTunnelProviderError.couldNotStartBackend)

            case .invalidState:
                // Must never happen
                fatalError()
            }
        }
    }

    override func stopTunnel(with reason: NEProviderStopReason, completionHandler: @escaping () -> Void) {
        wg_log(.info, staticMessage: "Stopping tunnel")

        adapter.stop { error in
            ErrorNotifier.removeLastErrorFile()

            if let error = error {
                wg_log(.error, message: "Failed to stop WireGuard adapter: \(error.localizedDescription)")
            }
            completionHandler()

            #if os(macOS)
            // HACK: This is a filthy hack to work around Apple bug 32073323 (dup'd by us as 47526107).
            // Remove it when they finally fix this upstream and the fix has been rolled out to
            // sufficient quantities of users.
            exit(0)
            #endif
        }
    }

    override func handleAppMessage(_ messageData: Data, completionHandler: ((Data?) -> Void)? = nil) {
        guard let completionHandler = completionHandler else { return }

        do {
            self.handleAppMessageInternal(try TunnelMessage(messageData), completionHandler: completionHandler)
        } catch {
            wg_log(.error, message: "Failed to decode message.")
            completionHandler(nil)
        }
    }

    private func handleAppMessageInternal(_ message: TunnelMessage, completionHandler: @escaping ((Data?) -> Void)) {
        wg_log(.info, message: "New message from main application: '\(message)'")

        switch message {
            case .getRuntimeConfiguration:
                adapter.getRuntimeConfiguration { settings in
                    var data: Data?
                    if let settings = settings {
                        data = settings.data(using: .utf8)!
                    }
                    completionHandler(data)
                }

            case .configurationSwitch(let newConfig):
                do {
                    let tunnelConfiguration = try TunnelConfiguration(fromWgQuickConfig: newConfig)
                    adapter.update(tunnelConfiguration: tunnelConfiguration) { error in
                        if let error = error {
                            wg_log(.error, message: "Failed to switch tunnel configuration: \(error.localizedDescription)")
                            completionHandler(nil)
                            return
                        }

                        self.adapter.getRuntimeConfiguration { settings in
                            var data: Data?
                            if let settings = settings {
                                data = settings.data(using: .utf8)!
                            }
                            completionHandler(data)
                        }
                    }
                } catch {
                    completionHandler(nil)
                }

            case .telemetryEnabledChanged(let flag):
                Glean.shared.setUploadEnabled(flag)
                completionHandler(nil)
        }
    }
}

extension WireGuardLogLevel {
    var osLogLevel: OSLogType {
        switch self {
        case .verbose:
            return .debug
        case .error:
            return .error
        }
    }
}
