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
    var interface:InterfaceConfiguration? = nil

    @objc init(privateKey: Data, ipv4Address: String, ipv6Address: String, closure: @escaping (Bool) -> Void) {
        super.init()

        Logger.configureGlobal(tagged: "APP", withFilePath: "")

        assert(privateKey.count == TunnelConfiguration.keyLength)

        interface = InterfaceConfiguration(privateKey: privateKey)
        if let ipv4Address = IPAddressRange(from: ipv4Address),
           let ipv6Address = IPAddressRange(from: ipv6Address) {
            interface!.addresses = [ipv4Address, ipv6Address]
        }

        NETunnelProviderManager.loadAllFromPreferences { [weak self] managers, error in
            if let self = self, error == nil {
                self.tunnel = managers?.first
                if self.tunnel == nil {
                    Logger.global?.log(message: "Creating the tunnel")
                    self.tunnel = NETunnelProviderManager()
                    closure(true)
                    return
                }

                Logger.global?.log(message: "Tunnel already exists")
                closure(true)
                return
            }

            Logger.global?.log(message: "Loading from preference failed.")
            closure(false)
        }
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

            self.tunnel!.loadFromPreferences { error in
                if let error = error {
                    Logger.global?.log(message: "Connect Tunnel Load Error: \(error)")
                    closure(false)
                    return
                }

                do {
                    try (self.tunnel!.connection as? NETunnelProviderSession)?.startTunnel()
                    closure(true)
                } catch {
                    Logger.global?.log(message: "Something went wrong");
                    closure(false)
                }
            }
        }
    }

    @objc func disconnect(closure: @escaping (Bool) -> Void) {
        Logger.global?.log(message: "Disconnecting")
        assert(tunnel != nil)
        (tunnel!.connection as? NETunnelProviderSession)?.stopTunnel()
        closure(true)
    }
}
