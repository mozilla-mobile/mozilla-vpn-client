/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import NetworkExtension

let VPN_NAME = "Mozilla VPN"

@objc class VPNIPAddressRange: NSObject {
    public var address: NSString = ""
    public var networkPrefixLength: UInt8 = 0
    public var isIpv6: Bool = false

    @objc init(address: NSString, networkPrefixLength: UInt8, isIpv6: Bool) {
        super.init()

        self.address = address
        self.networkPrefixLength = networkPrefixLength
        self.isIpv6 = isIpv6
    }
}

@objc class VPNServerData: NSObject {
    // This should be a struct, but we must use a class for objc interoperability
    let dns: String
    let ipv6Gateway: String
    let publicKey: String
    let ipv4AddrIn: String
    let port: Int

    @objc init(dns: String, ipv6Gateway: String, publicKey: String, ipv4AddrIn: String, port: Int) {
        self.dns = dns
        self.ipv6Gateway = ipv6Gateway
        self.publicKey = publicKey
        self.ipv4AddrIn = ipv4AddrIn
        self.port = port

        super.init()
    }

    var serverWithPort: String {
        return self.ipv4AddrIn + ":\(self.port )"
    }
}

public class IOSControllerImpl: NSObject {
    private static let logger = IOSLoggerImpl(tag: "IOSSwiftController")

    private var stateChangeCallback: ((Bool) -> Void?)? = nil
    private var privateKey : PrivateKey? = nil
    private var deviceIpv4Address: String? = nil
    private var deviceIpv6Address: String? = nil

    @objc enum ConnectionState: Int { case Error, Connected, Disconnected }

    @objc init(bundleID: String, privateKey: Data, deviceIpv4Address: String, deviceIpv6Address: String, closure: @escaping (ConnectionState, Date?) -> Void, callback: @escaping (Bool) -> Void) {
        super.init()

        stateChangeCallback = callback
        self.privateKey = PrivateKey(rawValue: privateKey)
        self.deviceIpv4Address = deviceIpv4Address
        self.deviceIpv6Address = deviceIpv6Address

        NotificationCenter.default.addObserver(self, selector: #selector(self.vpnStatusDidChange(notification:)), name: Notification.Name.NEVPNStatusDidChange, object: nil)

        TunnelManager.initialize(bundleID) { error, tunnel in
            if error != nil {
                closure(ConnectionState.Error, nil)
                return
            }

            if TunnelManager.session?.status == .connected {
                let failureClosure = {
                    // Fall back to iOS's connection date, which will be incorrect if there has been
                    // a user-initiated server switch or silent server switch.
                    closure(ConnectionState.Connected, TunnelManager.session?.connectedDate)
                }
                guard let session = TunnelManager.session else {
                    failureClosure()
                    return
                }
                do {
                    try session.sendProviderMessage(TunnelMessage.getConnectionTimestamp.encode()) { date in
                        guard let dateStampData = date else {
                            IOSControllerImpl.logger.error(message: "Missing data about datestamp")
                            failureClosure()
                            return
                        }
                        let timestamp = dateStampData.withUnsafeBytes({ $0.load(as: Double.self) })
                        closure(ConnectionState.Connected, Date(timeIntervalSinceReferenceDate: timestamp))
                    }
                } catch {
                    IOSControllerImpl.logger.error(message: "Failed to retrieve data about datestamp. \(error)")
                    failureClosure()
                }
            } else {
                closure(ConnectionState.Disconnected, nil)
            }
        }
    }

    @objc private func vpnStatusDidChange(notification: Notification) {
        guard let session = (notification.object as? NETunnelProviderSession), TunnelManager.session == session else { return }

        switch session.status {
        case .connected:
            IOSControllerImpl.logger.debug(message: "STATE CHANGED: connected")
        case .connecting:
            IOSControllerImpl.logger.debug(message: "STATE CHANGED: connecting")
        case .disconnected:
            IOSControllerImpl.logger.debug(message: "STATE CHANGED: disconnected")
        case .disconnecting:
            IOSControllerImpl.logger.debug(message: "STATE CHANGED: disconnecting")
        case .invalid:
            IOSControllerImpl.logger.debug(message: "STATE CHANGED: invalid")
        case .reasserting:
            IOSControllerImpl.logger.debug(message: "STATE CHANGED: reasserting")
        default:
            IOSControllerImpl.logger.debug(message: "STATE CHANGED: unknown status")
        }

        // We care about "unknown" state changes.
        if (session.status != .connected && session.status != .disconnected) {
            return
        }

        // If disconnected, we know for sure that this is true
        if (session.status == .disconnected) {
            stateChangeCallback?(false)
            return
        }

       // This timer is used to workaround a Schrödinger's cat bug: if we check
       // the connection status immediately when connected, we alter the iOS16
       // connectivity state and we break the VPN tunnel (intermittently). With
       // a timer this does not happen.
        _ = Timer.scheduledTimer(withTimeInterval: 0.5, repeats: false) {_ in
            self.monitorConnection()
        }
    }

    private func monitorConnection() -> Void {
        // Let's call `stateChangeCallback` if connected and if
        // last_handshake_time_sec is not 0.
        self.checkStatus { _, _, configString in
            if TunnelManager.session?.status != .connected { return; }

            let lines = configString.splitToArray(separator: "\n")
            if let line = lines.first(where: { $0.starts(with: "last_handshake_time_sec") }) {
                let parts = line.splitToArray(separator: "=")
                if parts.count > 1 && Int(parts[1]) ?? 0 > 0 {
                    self.stateChangeCallback?(true)
                    return
                }
            }

            _ = Timer.scheduledTimer(withTimeInterval: 0.5, repeats: false) { _ in
                self.monitorConnection()
            }
        }
    }

    @objc func connect(serverData: [VPNServerData], excludeLocalNetworks: Bool, allowedIPAddressRanges: [VPNIPAddressRange], reason: Int, gleanDebugTag: String, isSuperDooperFeatureActive: Bool, installationId: String, disconnectOnErrorCallback: @escaping () -> Void, onboardingCompletedCallback: @escaping () -> Void, vpnConfigPermissionResponseCallback: @escaping (Bool) -> Void) {
        IOSControllerImpl.logger.debug(message: "Connecting")

        TunnelManager.withTunnel { tunnel in
            // Let's remove the previous config if it exists.
            (tunnel.protocolConfiguration as? NETunnelProviderProtocol)?.destroyConfigurationReference()
            let configs: [TunnelConfiguration] = serverData.map({createConfig(for: $0, allowedIPAddressRanges: allowedIPAddressRanges)})

            guard let serverName = serverData.first?.serverWithPort else {
              IOSControllerImpl.logger.error(message: "No VPN server data found")
              disconnectOnErrorCallback()
              return
            }
            return self.configureTunnel(configs: configs, reason: reason, serverName: serverName, excludeLocalNetworks: excludeLocalNetworks, gleanDebugTag: gleanDebugTag, isSuperDooperFeatureActive: isSuperDooperFeatureActive, installationId: installationId, disconnectOnErrorCallback: disconnectOnErrorCallback, onboardingCompletedCallback: onboardingCompletedCallback, vpnConfigPermissionResponseCallback: vpnConfigPermissionResponseCallback)
        }
    }

    func configureTunnel(configs: [TunnelConfiguration], reason: Int, serverName: String, excludeLocalNetworks: Bool, gleanDebugTag: String, isSuperDooperFeatureActive: Bool, installationId: String, disconnectOnErrorCallback: @escaping () -> Void, onboardingCompletedCallback: @escaping () -> Void, vpnConfigPermissionResponseCallback: @escaping (Bool) -> Void) {
        TunnelManager.withTunnel { tunnel in
            guard let config = configs.first else {
              IOSControllerImpl.logger.error(message: "No VPN config found")
              disconnectOnErrorCallback()
              return
            }
            let proto = NETunnelProviderProtocol(tunnelConfiguration: config)
            proto!.providerBundleIdentifier = TunnelManager.vpnBundleId
            proto!.disconnectOnSleep = false
            proto!.serverAddress = serverName

            if #available(iOS 15.1, *) {
                IOSControllerImpl.logger.debug(message: "Activating includeAllNetworks")
                proto!.includeAllNetworks = true
                proto!.excludeLocalNetworks = excludeLocalNetworks

                if #available(iOS 16.4, *) {
                    // By default, APNs is excluded from the VPN tunnel on 16.4 and later. We want to include it.
                    proto!.excludeAPNs = false
                }
            }

            var customConfig = proto?.providerConfiguration ?? [:]
            customConfig["isSuperDooperFeatureActive"] = isSuperDooperFeatureActive
            customConfig["gleanDebugTag"] = gleanDebugTag
            customConfig["installationId"] = installationId

            for i in 0..<configs.count {
                customConfig["config\(i)"] = configs[i].asWgQuickConfig()
            }
            proto?.providerConfiguration = customConfig

            tunnel.protocolConfiguration = proto
            tunnel.localizedDescription = VPN_NAME
            tunnel.isEnabled = true

            // Create a rule so that the VPN always connects. This allows reconnection if
            // the device reboots or the network extension is stopped for an unexpected reason.
            let alwaysConnect = NEOnDemandRuleConnect()
            alwaysConnect.interfaceTypeMatch = .any
            tunnel.isOnDemandEnabled = true
            tunnel.onDemandRules = [alwaysConnect]

            return tunnel.saveToPreferences { saveError in
                // At this point, the user has made a selection on the system config permission modal to either allow or not allow
                // the vpn configuration to be created, so it is safe to run activation retries via Controller::startHandshakeTimer()
                // without the possibility or re-prompting (flickering) the modal while it is currently being displayed
                vpnConfigPermissionResponseCallback(saveError == nil)

                if let error = saveError {
                    IOSControllerImpl.logger.error(message: "Connect Tunnel Save Error: \(error)")
                    disconnectOnErrorCallback()
                    return
                }

               IOSControllerImpl.logger.info(message: "Saving the tunnel succeeded")

               tunnel.loadFromPreferences { error in
                    if let error = error {
                        IOSControllerImpl.logger.error(message: "Connect Tunnel Load Error: \(error)")
                        disconnectOnErrorCallback()
                        return
                    }

                    IOSControllerImpl.logger.info(message: "Loading the tunnel succeeded")

                    do {
                        if (reason == 1 /* ReasonSwitching */) {
                            let settings = config.asWgQuickConfig()
                            let message = TunnelMessage.configurationSwitch(settings)
                            IOSControllerImpl.logger.info(message: "Sending new message \(message)")
                            try TunnelManager.session?.sendProviderMessage(message.encode()) {_ in return}
                        } else {
                            try TunnelManager.session?.startTunnel(options: ["source":"app"])
                        }
                        // If `try` didn't throw, run onboarding callback. This callback only matters when onboarding.
                        onboardingCompletedCallback()
                    } catch let error {
                        IOSControllerImpl.logger.error(message: "Something went wrong: \(error)")
                        disconnectOnErrorCallback()
                        return
                    }
                }
            }
        }
    }

    @objc func disconnect() {
        IOSControllerImpl.logger.info(message: "Disconnecting")
        TunnelManager.withTunnel { tunnel in

            // Turn off auto-connect, otherwise it will immediately reconnect.
            tunnel.isOnDemandEnabled = false;
            tunnel.onDemandRules = []

            tunnel.saveToPreferences { saveError in
                if let error = saveError {
                    IOSControllerImpl.logger.error(message: "Disonnect tunnel save error: \(error)")
                }
                TunnelManager.session?.stopTunnel()
            }

            // Needs to return something, but this will be discarded.
            return true
        }
    }

    @objc func deleteOSTunnelConfig() {
      IOSControllerImpl.logger.info(message: "Removing tunnel from iOS System Preferences")
      TunnelManager.withTunnel { tunnel in
        tunnel.removeFromPreferences(completionHandler: { error in
          if let error = error {
            IOSControllerImpl.logger.info(message: "Error when removing tunnel \(error.localizedDescription)")
          }
        })
      }
    }

    @objc func checkStatus(callback: @escaping (String, String, String) -> Void) {
        IOSControllerImpl.logger.info(message: "Check status")

        TunnelManager.withTunnel { tunnel in
            let proto = tunnel.protocolConfiguration as? NETunnelProviderProtocol
            if proto == nil {
                callback("", "", "")
                return
            }

            let tunnelConfiguration = proto?.asTunnelConfiguration()
            if tunnelConfiguration == nil {
                callback("", "", "")
                return
            }

            let serverIpv4Gateway = tunnelConfiguration?.interface.dns[0].address
            if serverIpv4Gateway == nil {
                callback("", "", "")
                return
            }

            let deviceIpv4Address = tunnelConfiguration?.interface.addresses[0].address
            if deviceIpv4Address == nil {
                callback("", "", "")
                return
            }

            guard let session = TunnelManager.session
            else {
                callback("", "", "")
                return
            }

            do {
                let message = TunnelMessage.getRuntimeConfiguration;
                IOSControllerImpl.logger.info(message: "Sending new message \(message)");
                try session.sendProviderMessage(message.encode()) { [callback] data in
                    guard let data = data,
                          let configString = String(data: data, encoding: .utf8)
                    else {
                        IOSControllerImpl.logger.error(message: "Failed to convert data to string")
                        callback("", "", "")
                        return
                    }

                    callback("\(serverIpv4Gateway!)", "\(deviceIpv4Address!)", configString)
                }
            } catch {
                IOSControllerImpl.logger.error(message: "Failed to retrieve data from session. \(error)")
                callback("", "", "")
            }

            return
        }
    }

    @objc func silentServerSwitch() {
        guard let session = TunnelManager.session else {
            IOSControllerImpl.logger.info(message: "No tunnel session available to send silentServerSwitch message");
            return
        }

        let message = TunnelMessage.silentServerSwitch;
        IOSControllerImpl.logger.info(message: "Sending new message \(message)");
        do {
            try session.sendProviderMessage(message.encode()) { _ in
                // IMPORTANT: Must keep this log line (or other code) in this closure.
                // The closure is optional for `sendProviderMessage`. However, without using a closure, the message is
                // sent but not received by the network extention. Mysterious, but couldn't figure out why in a
                // reasonable amount of time.
                IOSControllerImpl.logger.info(message: "\(message) sent");
            }
        } catch {
            IOSControllerImpl.logger.error(message: "Failed to send message to session. \(error)")
        }
    }

    private func createConfig(for serverData: VPNServerData, allowedIPAddressRanges: [VPNIPAddressRange]) -> TunnelConfiguration {
        let keyData = PublicKey(base64Key: serverData.publicKey)!
        let dnsServerIP = IPv4Address(serverData.dns)
        let ipv6GatewayIP = IPv6Address(serverData.ipv6Gateway)

        var peerConfiguration = PeerConfiguration(publicKey: keyData)
        peerConfiguration.endpoint = Endpoint(from: serverData.ipv4AddrIn + ":\(serverData.port )")
        peerConfiguration.allowedIPs = []

        allowedIPAddressRanges.forEach {
            if (!$0.isIpv6) {
                peerConfiguration.allowedIPs.append(IPAddressRange(address: IPv4Address($0.address as String)!, networkPrefixLength: $0.networkPrefixLength))
            } else {
                peerConfiguration.allowedIPs.append(IPAddressRange(address: IPv6Address($0.address as String)!, networkPrefixLength: $0.networkPrefixLength))
            }
        }

        var peerConfigurations: [PeerConfiguration] = []
        peerConfigurations.append(peerConfiguration)

        var interface = InterfaceConfiguration(privateKey: self.privateKey!)

        if let ipv4Address = IPAddressRange(from: deviceIpv4Address!),
           let ipv6Address = IPAddressRange(from: deviceIpv6Address!) {
            interface.addresses = [ipv4Address, ipv6Address]
        }
        interface.dns = [DNSServer(address: dnsServerIP!), DNSServer(address: ipv6GatewayIP!)]

        return TunnelConfiguration(name: VPN_NAME, interface: interface, peers: peerConfigurations)
    }
}
