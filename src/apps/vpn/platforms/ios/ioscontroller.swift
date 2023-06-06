/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import NetworkExtension

let vpnName = "Mozilla VPN"
var vpnBundleID = "";

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

    private var tunnel: NETunnelProviderManager? = nil
    private var stateChangeCallback: ((Bool, Date?) -> Void?)? = nil
    private var privateKey : PrivateKey? = nil
    private var deviceIpv4Address: String? = nil
    private var deviceIpv6Address: String? = nil

    @objc enum ConnectionState: Int { case Error, Connected, Disconnected }

    @objc init(bundleID: String, privateKey: Data, deviceIpv4Address: String, deviceIpv6Address: String, closure: @escaping (ConnectionState, Date?) -> Void, callback: @escaping (Bool, Date?) -> Void) {
        super.init()

        Logger.configureGlobal(tagged: "APP", withFilePath: "")

        vpnBundleID = bundleID;
        precondition(!vpnBundleID.isEmpty)

        stateChangeCallback = callback
        self.privateKey = PrivateKey(rawValue: privateKey)
        self.deviceIpv4Address = deviceIpv4Address
        self.deviceIpv6Address = deviceIpv6Address

        NotificationCenter.default.addObserver(self, selector: #selector(self.vpnStatusDidChange(notification:)), name: Notification.Name.NEVPNStatusDidChange, object: nil)

        TunnelManager.initialize(bundleID) { error, tunnel in
            if (error != nil || tunnel == nil) {
                closure(ConnectionState.Error, nil)
                return
            }

            let tunnelConnectionInfo = (tunnel!.connection as? NETunnelProviderSession)
            if tunnelConnectionInfo?.status == .connected {
                closure(ConnectionState.Connected, tunnelConnectionInfo?.connectedDate)
            } else {
                closure(ConnectionState.Disconnected, nil)
            }
        } 
    }

    @objc private func vpnStatusDidChange(notification: Notification) {
        guard let session = (notification.object as? NETunnelProviderSession), TunnelManager.session == session else { return }

        switch session.status {
        case .connected:
            Logger.global?.log(message: "STATE CHANGED: connected")
        case .connecting:
            Logger.global?.log(message: "STATE CHANGED: connecting")
        case .disconnected:
            Logger.global?.log(message: "STATE CHANGED: disconnected")
        case .disconnecting:
            Logger.global?.log(message: "STATE CHANGED: disconnecting")
        case .invalid:
            Logger.global?.log(message: "STATE CHANGED: invalid")
        case .reasserting:
            Logger.global?.log(message: "STATE CHANGED: reasserting")
        default:
            Logger.global?.log(message: "STATE CHANGED: unknown status")
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
        TunnelManager.checkStatus { _, _, configString in
            if TunnelManager.session?.status != .connected { return; }

            let lines = configString.splitToArray(separator: "\n")
            if let line = lines.first(where: { $0.starts(with: "last_handshake_time_sec") }) {
                let parts = line.splitToArray(separator: "=")
                if parts.count > 1 && Int(parts[1]) ?? 0 > 0 {
                    self.stateChangeCallback?(true, self.tunnel?.connection.connectedDate)
                    return
                }
            }

            _ = Timer.scheduledTimer(withTimeInterval: 0.5, repeats: false) { _ in
                self.monitorConnection()
            }
        }
    }

    @objc func connect(dnsServer: String, serverIpv6Gateway: String, serverPublicKey: String, serverIpv4AddrIn: String, serverPort: Int,  allowedIPAddressRanges: Array<VPNIPAddressRange>, reason: Int, failureCallback: @escaping () -> Void) {
        Logger.global?.log(message: "Connecting")
        
        // Let's remove the previous config if it exists.
        TunnelManager.protocolConfiguration?.destroyConfigurationReference()

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

        let config = TunnelConfiguration(name: vpnName, interface: interface, peers: peerConfigurations)

        return self.configureTunnel(config: config, reason: reason, serverName: serverIpv4AddrIn + ":\(serverPort )", failureCallback: failureCallback)
    }

    func configureTunnel(config: TunnelConfiguration, reason: Int, serverName: String, failureCallback: @escaping () -> Void) {
        let proto = NETunnelProviderProtocol(tunnelConfiguration: config)
        proto!.providerBundleIdentifier = vpnBundleID
        proto!.disconnectOnSleep = false
        proto!.serverAddress = serverName;

        if #available(iOS 15.1, *) {
            Logger.global?.log(message: "Activating includeAllNetworks")
            proto!.includeAllNetworks = true
            proto!.excludeLocalNetworks = true
        }
        
        _ = TunnelManager.withTunnel { tunnel in
            tunnel.protocolConfiguration = proto
            tunnel.localizedDescription = vpnName
            tunnel.isEnabled = true

            return tunnel.saveToPreferences { saveError in
                if let error = saveError {
                    Logger.global?.log(message: "Connect Tunnel Save Error: \(error)")
                    failureCallback()
                    return
                }

                Logger.global?.log(message: "Saving the tunnel succeeded")

                tunnel.loadFromPreferences { error in
                    if let error = error {
                        Logger.global?.log(message: "Connect Tunnel Load Error: \(error)")
                        failureCallback()
                        return
                    }

                    Logger.global?.log(message: "Loading the tunnel succeeded")

                    do {
                        if (reason == 1 /* ReasonSwitching */) {
                            TunnelMessage
                                .configurationSwitch(config.asWgQuickConfig())
                                .send()
                        } else {
                            try TunnelManager.session?.startTunnel()
                        }
                    } catch let error {
                        Logger.global?.log(message: "Something went wrong: \(error)")
                        failureCallback()
                        return
                    }
                }
            }
        }
    }

    @objc func disconnect() {
        Logger.global?.log(message: "Disconnecting")
        TunnelManager.session?.stopTunnel()
    }

    @objc func checkStatus(callback: @escaping (String, String, String) -> Void) {
        Logger.global?.log(message: "Check status")
        TunnelManager.checkStatus(callback)
    }
}
