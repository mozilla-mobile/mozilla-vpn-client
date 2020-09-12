//
//  macoscontroller.swift
//  MozillaVPN
//
//  Created by Andrea Marchesini on 11/09/2020.
//

import Foundation

extension TunnelsManager {
    var selectedTunnel : TunnelContainer? {
        return numberOfTunnels() > 0 ? tunnel(at: 0) : nil
    }
}
public class MacOSControllerImpl : NSObject {

    var tunnelsManager: TunnelsManager? = nil
    var interface:InterfaceConfiguration? = nil

    @objc init(privateKey: Data, ipv4Address: String, ipv6Address: String, closure: @escaping (Bool) -> Void) {
        super.init();

        assert(privateKey.count == TunnelConfiguration.keyLength)

        interface = InterfaceConfiguration(privateKey: privateKey);
        if let ipv4Address = IPAddressRange(from: ipv4Address),
           let ipv6Address = IPAddressRange(from: ipv6Address) {
            interface!.addresses = [ipv4Address, ipv6Address]
        }

        TunnelsManager.create { [weak self] result in
            guard let self = self else { return }

            switch result {
            case .failure(let error):
                print("Creation of the tunnelsManagar failed", error.alertText);
                closure(false);
                return;

            case .success(let tunnelsManager):
                self.tunnelsManager = tunnelsManager;
                self.createDummyTunnel(closure: closure);
            }
        }
    }

    func createDummyTunnel(closure: @escaping (Bool) -> Void) {
        let peerConfigurations: [PeerConfiguration] = [];

        let config = TunnelConfiguration(name: "MozillaVPN", interface: interface!, peers: peerConfigurations);

        tunnelsManager!.add(tunnelConfiguration: config) { result in
            switch result {
            case .failure(let error):
                print("TunnelsManager.add failed", error.alertText);
                closure(false);
            case .success:
                closure(true);
            }
        }
    }

    @objc func connect(closure: @escaping (Bool) -> Void) {
        print("Connecting!");
        assert(tunnelsManager != nil);

        guard let tunnel = tunnelsManager!.selectedTunnel else { return };
        if tunnel.status == .inactive {
            tunnelsManager?.startActivation(of: tunnel)
        } else if tunnel.status == .active {
            tunnelsManager?.startDeactivation(of: tunnel)
        }

        closure(true);
    }
}
