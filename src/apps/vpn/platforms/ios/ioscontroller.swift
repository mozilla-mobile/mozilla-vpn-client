/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import NetworkExtension

let VPN_NAME = "Mozilla VPN"

@objc class VPNIPAddressRange : NSObject {
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

public class IOSControllerImpl : NSObject {
    private static let logger = IOSLoggerImpl(tag: "IOSController")

    private var stateChangeCallback: ((Bool, Date?) -> Void?)? = nil
    private var privateKey : PrivateKey? = nil
    private var deviceIpv4Address: String? = nil
    private var deviceIpv6Address: String? = nil

    @objc enum ConnectionState: Int { case Error, Connected, Disconnected }

    @objc init(bundleID: String, privateKey: Data, deviceIpv4Address: String, deviceIpv6Address: String, closure: @escaping (ConnectionState, Date?) -> Void, callback: @escaping (Bool, Date?) -> Void) {
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
                closure(ConnectionState.Connected, TunnelManager.session?.connectedDate)
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
            stateChangeCallback?(false, nil)
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
                    self.stateChangeCallback?(true, TunnelManager.session?.connectedDate)
                    return
                }
            }

            _ = Timer.scheduledTimer(withTimeInterval: 0.5, repeats: false) { _ in
                self.monitorConnection()
            }
        }
    }

    @objc func connect(dnsServer: String, serverIpv6Gateway: String, serverPublicKey: String, serverIpv4AddrIn: String, serverPort: Int,  allowedIPAddressRanges: Array<VPNIPAddressRange>, reason: Int, failureCallback: @escaping () -> Void) {
        IOSControllerImpl.logger.debug(message: "Connecting")
        
        let _ = TunnelManager.withTunnel { tunnel in
            // Let's remove the previous config if it exists.
            (tunnel.protocolConfiguration as? NETunnelProviderProtocol)?.destroyConfigurationReference()

            let keyData = PublicKey(base64Key: serverPublicKey)!
            let dnsServerIP = IPv4Address(dnsServer)
            let ipv6GatewayIP = IPv6Address(serverIpv6Gateway)

            var peerConfiguration = PeerConfiguration(publicKey: keyData)
            peerConfiguration.endpoint = Endpoint(from: serverIpv4AddrIn + ":\(serverPort )")
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

            var interface = InterfaceConfiguration(privateKey: privateKey!)

            if let ipv4Address = IPAddressRange(from: deviceIpv4Address!),
               let ipv6Address = IPAddressRange(from: deviceIpv6Address!) {
                interface.addresses = [ipv4Address, ipv6Address]
            }
            interface.dns = [DNSServer(address: dnsServerIP!), DNSServer(address: ipv6GatewayIP!)]

            let config = TunnelConfiguration(name: VPN_NAME, interface: interface, peers: peerConfigurations)

            return self.configureTunnel(config: config, reason: reason, serverName: serverIpv4AddrIn + ":\(serverPort )", failureCallback: failureCallback)
        }
    }

    func configureTunnel(config: TunnelConfiguration, reason: Int, serverName: String, failureCallback: @escaping () -> Void) {
        let _ = TunnelManager.withTunnel { tunnel in
            let proto = NETunnelProviderProtocol(tunnelConfiguration: config)
            proto!.providerBundleIdentifier = TunnelManager.vpnBundleId
            proto!.disconnectOnSleep = false
            proto!.serverAddress = serverName;

            if #available(iOS 15.1, *) {
                IOSControllerImpl.logger.debug(message: "Activating includeAllNetworks")
                proto!.includeAllNetworks = true
                proto!.excludeLocalNetworks = true

                if #available(iOS 16.4, *) {
                    // By default, APNs is excluded from the VPN tunnel on 16.4 and later. We want to include it.
                    proto!.excludeAPNs = false
                }
            }

            tunnel.protocolConfiguration = proto
            tunnel.localizedDescription = VPN_NAME
            tunnel.isEnabled = true

            return tunnel.saveToPreferences { saveError in
                if let error = saveError {
                    IOSControllerImpl.logger.error(message: "Connect Tunnel Save Error: \(error)")
                    failureCallback()
                    return
                }

                IOSControllerImpl.logger.info(message: "Saving the tunnel succeeded")

               tunnel.loadFromPreferences { error in
                    if let error = error {
                        IOSControllerImpl.logger.error(message: "Connect Tunnel Load Error: \(error)")
                        failureCallback()
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
                            try TunnelManager.session?.startTunnel()
                        }
                    } catch let error {
                        IOSControllerImpl.logger.error(message: "Something went wrong: \(error)")
                        failureCallback()
                        return
                    }
                }
            }
        }
    }

    @objc func disconnect() {
        IOSControllerImpl.logger.info(message: "Disconnecting")
        TunnelManager.session?.stopTunnel()
    }

    @objc func checkStatus(callback: @escaping (String, String, String) -> Void) {
        IOSControllerImpl.logger.info(message: "Check status")
        
        let _ = TunnelManager.withTunnel { tunnel in
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
}
