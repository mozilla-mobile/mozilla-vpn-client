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

        Logger.configureGlobal(tagged: "APP", withFilePath: FileManager.logFileURL?.path)

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
                    Logger.global?.log(message: "Creating the dummy tunnel")
                    self.createDummyTunnel(closure: closure)
                    return
                }

                Logger.global?.log(message: "Dummy channel already exists")
                closure(true)
                return
            }

            Logger.global?.log(message: "Loading from preference failed.")
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
                Logger.global?.log(message: "Connect Tunnel Save Error: \(error)")
                closure(false)
                return
            }

            self.tunnel?.loadFromPreferences { error in
                if let error = error {
                    Logger.global?.log(message: "Connect Tunnel Load Error: \(error)")
                    closure(false)
                    return
                }

                self.tunnel = tunnelProviderManager
                closure(true)
            }
        }
    }

    @objc func connect(closure: @escaping (Bool) -> Void) {
        Logger.global?.log(message: "Connecting! Tunnel: \(tunnel)")
        assert(tunnel != nil)

        do {
            try (tunnel!.connection as? NETunnelProviderSession)?.startTunnel()
            closure(true)
        } catch {
            closure(false)
        }
    }
}
