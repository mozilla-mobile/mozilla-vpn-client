//
//  macoscontroller.swift
//  MozillaVPN
//
//  Created by Andrea Marchesini on 11/09/2020.
//

import Foundation
import NetworkExtension

public class MacOSControllerImpl : NSObject {

    private var tunnel: NETunnelProviderManager? = nil
    private var observer: NSObjectProtocol? = nil
    private var externalStateChangeCallback: ((Bool) -> Void?)? = nil;
    var interface:InterfaceConfiguration? = nil

    @objc enum ConnectionState: Int { case Error, Connected, Disconnected }

    @objc init(privateKey: Data, ipv4Address: String, ipv6Address: String, closure: @escaping (ConnectionState) -> Void, externalCallback: @escaping (Bool) -> Void) {
        super.init()

        assert(privateKey.count == TunnelConfiguration.keyLength)

        Logger.configureGlobal(tagged: "APP", withFilePath: "")

        externalStateChangeCallback = externalCallback;

        NotificationCenter.default.addObserver(self, selector: #selector(self.vpnStatusDidChange(notification:)), name: Notification.Name.NEVPNStatusDidChange, object: nil)

        interface = InterfaceConfiguration(privateKey: privateKey)
        if let ipv4Address = IPAddressRange(from: ipv4Address),
           let ipv6Address = IPAddressRange(from: ipv6Address) {
            interface!.addresses = [ipv4Address, ipv6Address]
        }

        NETunnelProviderManager.loadAllFromPreferences { [weak self] managers, error in
            if let error = error {
                Logger.global?.log(message: "Loading from preference failed: \(error)")
                closure(ConnectionState.Error)
                return
            }

            if self == nil {
                Logger.global?.log(message: "We are shutting down.")
                return;
            }

            let tunnel = managers?.first
            if tunnel == nil {
                Logger.global?.log(message: "Creating the tunnel")
                self!.tunnel = NETunnelProviderManager()
                closure(ConnectionState.Disconnected)
                return
            }

            Logger.global?.log(message: "Tunnel already exists")

            let proto = tunnel!.protocolConfiguration as? NETunnelProviderProtocol
            if proto == nil {
                tunnel!.removeFromPreferences { _ in }

                Logger.global?.log(message: "Creating the tunnel because its proto is invalid")
                self!.tunnel = NETunnelProviderManager()
                closure(ConnectionState.Disconnected)
                return
            }

            self!.tunnel = tunnel
            if tunnel?.connection.status == .connected {
                closure(ConnectionState.Connected)
            } else {
                closure(ConnectionState.Disconnected)
            }
        }
    }

    @objc private func vpnStatusDidChange(notification: Notification) {
        guard let session = (notification.object as? NETunnelProviderSession), tunnel?.connection == session else { return }

        switch session.status {
        case .connected:
            Logger.global?.log(message:"STATE CHANGED: connected")
        case .connecting:
            Logger.global?.log(message:"STATE CHANGED: connecting")
        case .disconnected:
            Logger.global?.log(message:"STATE CHANGED: disconnected")
        case .disconnecting:
            Logger.global?.log(message:"STATE CHANGED: disconnecting")
        case .invalid:
            Logger.global?.log(message:"STATE CHANGED: invalid")
        case .reasserting:
            Logger.global?.log(message:"STATE CHANGED: reasserting")
        default:
            Logger.global?.log(message:"STATE CHANGED: unknown status")
        }

        // We care about "unknown" state changes.
        if observer != nil || (session.status != .connected && session.status != .disconnected) {
            Logger.global?.log(message: "A");
            return;
        }

        externalStateChangeCallback?(session.status == .connected)
    }

    @objc func connect(serverIpv4Gateway: String, serverIpv6Gateway: String, serverPublicKey: String, serverIpv4AddrIn: String, serverPort: Int, closure: @escaping (Bool) -> Void) {
        Logger.global?.log(message: "Connecting")
        assert(tunnel != nil)

        let keyData = Data(base64Key: serverPublicKey)!
        let ipv4GatewayIP = IPv4Address(serverIpv4Gateway)
        let ipv6GatewayIP = IPv6Address(serverIpv6Gateway)

        var peerConfiguration = PeerConfiguration(publicKey: keyData)
        peerConfiguration.endpoint = Endpoint(from: serverIpv4AddrIn + ":\(serverPort )")
        peerConfiguration.allowedIPs = [
            IPAddressRange(address: IPv4Address("0.0.0.0")!, networkPrefixLength: 0),
            IPAddressRange(address: IPv6Address("::")!, networkPrefixLength: 0)
        ]

        var peerConfigurations: [PeerConfiguration] = []
        peerConfigurations.append(peerConfiguration)

        interface!.dns = [ DNSServer(address: ipv4GatewayIP!), DNSServer(address: ipv6GatewayIP!) ]

        let config = TunnelConfiguration(name: "MozillaVPN", interface: interface!, peers: peerConfigurations)

        tunnel!.protocolConfiguration = NETunnelProviderProtocol(tunnelConfiguration: config)

        tunnel!.saveToPreferences { [unowned self] saveError in
            if let error = saveError {
                Logger.global?.log(message: "Connect Tunnel Save Error: \(error)")
                closure(false)
                return
            }

            Logger.global?.log(message: "Saving the tunnel succeeded");

            self.tunnel!.loadFromPreferences { error in
                if let error = error {
                    Logger.global?.log(message: "Connect Tunnel Load Error: \(error)")
                    closure(false)
                    return
                }

                Logger.global?.log(message: "Loading the tunnel succeeded");

                assert(observer == nil)
                observer = NotificationCenter.default.addObserver(forName: Notification.Name.NEVPNStatusDidChange, object: nil, queue: OperationQueue.main) { [self, closure]
                     (notification) in
                    guard let session = (notification.object as? NETunnelProviderSession), self.tunnel?.connection == session else { return }

                    if (session.status == .connected) {
                        NotificationCenter.default.removeObserver(observer!)
                        closure(true);
                    }

                    if (session.status == .disconnected ||
                        session.status == .disconnecting ||
                        session.status == .invalid) {
                        NotificationCenter.default.removeObserver(observer!)
                        observer = nil;
                        closure(false)
                    }
                }

                do {
                    try (self.tunnel!.connection as? NETunnelProviderSession)?.startTunnel()
                } catch let error {
                    Logger.global?.log(message: "Something went wrong: \(error)");
                    observer = nil
                    closure(false)
                    return;
                }
            }
        }
    }

    @objc func disconnect(closure: @escaping (Bool) -> Void) {
        Logger.global?.log(message: "Disconnecting")
        assert(tunnel != nil)

        assert(observer == nil)
        observer = NotificationCenter.default.addObserver(forName: Notification.Name.NEVPNStatusDidChange, object: nil, queue: OperationQueue.main) { [self, closure]
             (notification) in
            guard let session = (notification.object as? NETunnelProviderSession), tunnel?.connection == session else { return }

            if (session.status == .disconnected) {
                NotificationCenter.default.removeObserver(observer!)
                observer = nil;
                closure(true);
            }
        }

        (tunnel!.connection as? NETunnelProviderSession)?.stopTunnel()
    }
}
