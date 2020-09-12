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

        assert(privateKey.count == TunnelConfiguration.keyLength)

        interface = InterfaceConfiguration(privateKey: privateKey)
        if let ipv4Address = IPAddressRange(from: ipv4Address),
           let ipv6Address = IPAddressRange(from: ipv6Address) {
            interface!.addresses = [ipv4Address, ipv6Address]
        }

        NETunnelProviderManager.loadAllFromPreferences { [weak self] managers, error in
            if error != nil {
                closure(false)
                return
            }

            if let self = self, error == nil {
                self.tunnel = managers?.first
                if self.tunnel == nil {
                    self.createDummyTunnel(closure: closure)
                    return
                }
                closure(true)
                return
            }

            print("Loading from preference failed.")
            closure(false)
        }
    }

    func createDummyTunnel(closure: @escaping (Bool) -> Void) {
        let peerConfigurations: [PeerConfiguration] = []

        let config = TunnelConfiguration(name: "MozillaVPN", interface: interface!, peers: peerConfigurations)

        let tunnelProviderManager = NETunnelProviderManager()
        tunnelProviderManager.protocolConfiguration = NETunnelProviderProtocol(tunnelConfiguration: config)

        tunnelProviderManager.saveToPreferences { [unowned self] saveError in
            if let error = saveError {
                self.tunnel = nil
                Logger.global?.log(message: "Connect Tunnel Save Error: \(error)")
                closure(false)
                return
            }

            self.tunnel = tunnelProviderManager
            self.tunnel?.loadFromPreferences { error in
                if let error = error {
                    Logger.global?.log(message: "Connect Tunnel Load Error: \(error)")
                    closure(false)
                    return
                }

                closure(true)
            }
        }
    }

    @objc func connect(closure: @escaping (Bool) -> Void) {
        print("Connecting!")
        assert(tunnel != nil)

        do {
            try (tunnel!.connection as? NETunnelProviderSession)?.startTunnel()
            closure(true)
        } catch {
            closure(false)
        }
    }
}
