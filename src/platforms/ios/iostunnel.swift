// SPDX-License-Identifier: MIT
// Copyright © 2018-2020 WireGuard LLC. All Rights Reserved.

import Foundation
import NetworkExtension
import os
import WidgetKit

class PacketTunnelProvider: NEPacketTunnelProvider, SilentServerSwitching {
    private let logger = IOSLoggerImpl(tag: "Tunnel")

    private let connectionHealthMonitor = ConnectionHealth()

    private var originalStartTime: Date?

    private var currentServerConfig = 0

    private lazy var adapter: WireGuardAdapter = {
        return WireGuardAdapter(with: self) { [self] logLevel, message in
            switch logLevel {
            case .verbose:
                return logger.debug(message: message)
            case .error:
                return logger.error(message: message)
            }
        }
    }()

    override init() {
        super.init()
        connectionHealthMonitor.serverSwitchingDelegate = self
    }

    override func startTunnel(options: [String: NSObject]?, completionHandler: @escaping (Error?) -> Void) {
        let errorNotifier = ErrorNotifier(activationAttemptId: nil)
        let isSourceApp = ((options?["source"] as? String) ?? "") == "app"
        logger.info(message: "Starting tunnel from the " + (isSourceApp ? "app" : "OS directly, rather than the app"))

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

                self.logger.info(message: "Tunnel interface is \(interfaceName)")

                self.originalStartTime = Date()

                if let endpointHost = tunnelConfiguration.peers.first?.endpoint?.host {
                    self.connectionHealthMonitor.start(for: String(describing: endpointHost))
                } else {
                    // Intentionally using an assertion failure here without an early return.
                    // This is new functionality being added for connection health checks.
                    // It would be surprising if this was ever nil (and we hit this block).
                    // If on a debug build, we want to halt everything, done with assertionFailure.
                    // But if this has been occuring on prod already, we don't want to change
                    // behavior to something that halts the tunnel setup.
                    self.logger.info(message: "Tunnel config is missing endpoint")
                    assertionFailure("Missing endpoint")
                }

                WidgetCenter.shared.reloadAllTimelines()
                if #available(iOS 18.0, *) {
                    ControlCenter.shared.reloadAllControls()
                }
                completionHandler(nil)
                return
            }

            switch adapterError {
            case .cannotLocateTunnelFileDescriptor:
                self.logger.error(message: "Starting tunnel failed: could not determine file descriptor")
                errorNotifier.notify(PacketTunnelProviderError.couldNotDetermineFileDescriptor)
                completionHandler(PacketTunnelProviderError.couldNotDetermineFileDescriptor)

            case .dnsResolution(let dnsErrors):
                let hostnamesWithDnsResolutionFailure = dnsErrors.map { $0.address }
                    .joined(separator: ", ")
                self.logger.error(message: "DNS resolution failed for the following hostnames: \(hostnamesWithDnsResolutionFailure)")
                errorNotifier.notify(PacketTunnelProviderError.dnsResolutionFailure)
                completionHandler(PacketTunnelProviderError.dnsResolutionFailure)

            case .setNetworkSettings(let error):
                self.logger.error(message: "Starting tunnel failed with setTunnelNetworkSettings returning \(error.localizedDescription)")
                errorNotifier.notify(PacketTunnelProviderError.couldNotSetNetworkSettings)
                completionHandler(PacketTunnelProviderError.couldNotSetNetworkSettings)

            case .startWireGuardBackend(let errorCode):
                self.logger.error(message: "Starting tunnel failed with wgTurnOn returning \(errorCode)")
                errorNotifier.notify(PacketTunnelProviderError.couldNotStartBackend)
                completionHandler(PacketTunnelProviderError.couldNotStartBackend)

            case .invalidState:
                // Must never happen
                self.logger.error(message: "Starting tunnel failed with invalidState")
                fatalError()
            }
        }
    }

    override func stopTunnel(with reason: NEProviderStopReason, completionHandler: @escaping () -> Void) {
        logger.info(message: "Stopping tunnel")

        adapter.stop { error in
            ErrorNotifier.removeLastErrorFile()
            self.connectionHealthMonitor.stop()

            if let error = error {
                self.logger.error(message: "Failed to stop WireGuard adapter: \(error.localizedDescription)")
            }
            WidgetCenter.shared.reloadAllTimelines()
            if #available(iOS 18.0, *) {
                ControlCenter.shared.reloadAllControls()
            }
            completionHandler()
        }
    }

    override func handleAppMessage(_ messageData: Data, completionHandler: ((Data?) -> Void)? = nil) {
        guard let completionHandler = completionHandler else { return }
        
        do {
            let message = try TunnelMessage(messageData)
            logger.info(message: "Received new message: \(message)")
            
            switch message {
            case .getRuntimeConfiguration:
                adapter.getRuntimeConfiguration { settings in
                    var data: Data?
                    if let settings = settings {
                        data = settings.data(using: .utf8)!
                    }
                    completionHandler(data)
                }

            case .getConnectionTimestamp:
                let data = withUnsafeBytes(of: originalStartTime?.timeIntervalSinceReferenceDate) { Data($0) }
                completionHandler(data)
            case .configurationSwitch(let configString):
                // Updates the tunnel configuration and responds with the active configuration
                logger.info(message: "Switching tunnel configuration from app message")
                updateServerConfig(with: configString, completionHandler: completionHandler)
            case .silentServerSwitch:
                silentServerSwitch()
                completionHandler(nil)
            }
        } catch {
            logger.error(message: "Unexpected error while parsing message: \(error).")
            completionHandler(nil)
        }
    }

    private func updateServerConfig(with configString: String, completionHandler: ((Data?) -> Void)? = nil) {
        do {
            let tunnelConfiguration = try TunnelConfiguration(fromWgQuickConfig: configString)
            adapter.update(tunnelConfiguration: tunnelConfiguration) { error in
                if let error = error {
                    self.logger.error(message: "Failed to switch tunnel configuration: \(error.localizedDescription)")
                    completionHandler?(nil)
                    return
                }
            }
        } catch {
          completionHandler?(nil)
        }
    }

    func silentServerSwitch() {
        if let fallbackServerConfig = nextValidServerConfig() {
            self.logger.info(message: "Sending silent server switch in Network Extension")
            updateServerConfig(with: fallbackServerConfig)
            (protocolConfiguration as? NETunnelProviderProtocol)?.providerConfiguration?["fallbackConfig"] = ""
        } else {
            logger.info(message: "Silent server switch called, but no fallbackConfig available")
        }
    }

    private func nextValidServerConfig() -> String? {
        // Return the next valid config. If there is only one server config (the current one), return it to reconnect to that sole
        // available server.
        guard let providerConfig = (protocolConfiguration as? NETunnelProviderProtocol)?.providerConfiguration, let serverConfigs = providerConfig["configs"] as? [String] else {
            logger.error(message: "No protocol config found")
            return nil
        }
        // Potential future work: Use server weights when selecting next server here and in iOS.
        let numberOfServers = serverConfigs.count
        currentServerConfig = (currentServerConfig + 1) % numberOfServers
        return serverConfigs[currentServerConfig]
    }
}
