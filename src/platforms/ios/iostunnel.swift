// SPDX-License-Identifier: MIT
// Copyright © 2018-2020 WireGuard LLC. All Rights Reserved.

import Foundation
import NetworkExtension
import os
import IOSGlean

class PacketTunnelProvider: NEPacketTunnelProvider {
    private let logger = IOSLoggerImpl(tag: "Tunnel")

    private let connectionHealthMonitor = ConnectionHealth()

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

    private var gleanDebugTag: String? {
        guard let tag = (protocolConfiguration as? NETunnelProviderProtocol)?.providerConfiguration?["gleanDebugTag"] as? String,
            !tag.isEmpty else {
            return nil
        }
        return tag
    }

    private var shouldSendTelemetry: Bool {
        return ((protocolConfiguration as? NETunnelProviderProtocol)?.providerConfiguration?["isSuperDooperFeatureActive"] as? Bool) ?? false
    }

    override init() {
        super.init()

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
    }

    override func startTunnel(options: [String: NSObject]?, completionHandler: @escaping (Error?) -> Void) {
        // If this isGleanDebugTagActive check is in intializer, protocolConfig isn't set and it always fails.
        if let gleanDebugTag = gleanDebugTag {
            logger.info(message: "Setting Glean debug tag for Network Extension.")
            Glean.shared.setDebugViewTag(gleanDebugTag)
        }

        let errorNotifier = ErrorNotifier(activationAttemptId: nil)
        let isSourceApp = ((options?["source"] as? String) ?? "") == "app"
        logger.info(message: "Starting tunnel from the " + (isSourceApp ? "app" : "OS directly, rather than the app"))

        guard let tunnelProviderProtocol = self.protocolConfiguration as? NETunnelProviderProtocol,
              let tunnelConfiguration = tunnelProviderProtocol.asTunnelConfiguration() else {
            errorNotifier.notify(PacketTunnelProviderError.savedProtocolConfigurationIsInvalid)
            completionHandler(PacketTunnelProviderError.savedProtocolConfigurationIsInvalid)
            return
        }

        if shouldSendTelemetry {
            if let installationIdString = ((protocolConfiguration as? NETunnelProviderProtocol)?.providerConfiguration?["installationId"] as? String),
                let installationId = UUID(uuidString: installationIdString) {
                logger.info(message: "Setting installation ID in network extension")
                GleanMetrics.Session.installationId.set(installationId)
            } else {
                logger.error(message: "No installation ID found for network extension Glean instance")
            }

            GleanMetrics.Pings.shared.daemonsession.submit(reason: .daemonFlush)
        }

        // Start the tunnel
        adapter.start(tunnelConfiguration: tunnelConfiguration) { adapterError in
            guard let adapterError = adapterError else {
                let interfaceName = self.adapter.interfaceName ?? "unknown"

                self.logger.info(message: "Tunnel interface is \(interfaceName)")

                if self.shouldSendTelemetry {
                    GleanMetrics.Session.daemonSessionSource.set(isSourceApp ? "app" : "system")
                    GleanMetrics.Session.daemonSessionId.generateAndSet()
                    GleanMetrics.Session.daemonSessionStart.set()
                    GleanMetrics.Pings.shared.daemonsession.submit(reason: .daemonStart)
                }

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
                fatalError()
            }
        }
    }

    override func stopTunnel(with reason: NEProviderStopReason, completionHandler: @escaping () -> Void) {
        logger.info(message: "Stopping tunnel")

        adapter.stop { error in
            ErrorNotifier.removeLastErrorFile()
            self.connectionHealthMonitor.stop()
            if self.shouldSendTelemetry {
                GleanMetrics.Session.daemonSessionEnd.set()
                GleanMetrics.Pings.shared.daemonsession.submit(reason: .daemonEnd)

                // We are rotating the UUID here as a safety measure. It is rotated
                // again before the next session start, and we expect to see the
                // UUID created here in only one ping: The daemon ping with a
                // "flush" reason, which should contain this UUID and no other
                // metrics.
                GleanMetrics.Session.daemonSessionId.generateAndSet()
            }

            if let error = error {
                self.logger.error(message: "Failed to stop WireGuard adapter: \(error.localizedDescription)")
            }

            // Wait for all ping submission to be finished before continuing.
            // Very shortly after the completionHandler is called, iOS kills
            // the network extension. If this Glean.shared.shutdown() line is
            // in this class's deinit, the NE is killed before it can record
            // the ping. Thus, it MUST be before the completionHandler is
            // called.
            // Note: This doesn't wait for pings to be uploaded,
            // just for Glean to persist the ping for later sending.
            Glean.shared.shutdown();
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
            case .configurationSwitch(let configString):
                // Updates the tunnel configuration and responds with the active configuration
                logger.info(message: "Switching tunnel configuration")

                do {
                  let tunnelConfiguration = try TunnelConfiguration(fromWgQuickConfig: configString)
                  adapter.update(tunnelConfiguration: tunnelConfiguration) { error in
                    if let error = error {
                        self.logger.error(message: "Failed to switch tunnel configuration: \(error.localizedDescription)")
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
            case .telemetryEnabledChanged(let uploadEnabled):
                Glean.shared.setUploadEnabled(uploadEnabled)
                completionHandler(nil)
            }
        } catch {
            logger.error(message: "Unexpected error while parsing message: \(error).")
            completionHandler(nil)
        }
    }
}
