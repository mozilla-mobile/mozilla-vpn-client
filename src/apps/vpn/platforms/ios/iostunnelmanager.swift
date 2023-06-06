/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import NetworkExtension

// Singleton that manages the VPN tunnel and exposes functions
// for interacting with it.
class TunnelManager {
    private static var VPN_BUNDLE_ID = ""
    private static let VPN_NAME = "Mozilla VPN"
    
    private static let instance = TunnelManager()

    private var tunnel: Result<NETunnelProviderManager, Error>?
    
    static var session: NETunnelProviderSession? {
        get {
            return withTunnel { tunnel in
                return (tunnel.connection as? NETunnelProviderSession)!
            } as! NETunnelProviderSession?
        }
    }
    
    static var protocolConfiguration: NETunnelProviderProtocol? {
        get {
            return withTunnel { tunnel in
                return (tunnel.protocolConfiguration as? NETunnelProviderProtocol)!
            } as! NETunnelProviderProtocol?
        }
    }

    private init() {}
    
    static func initialize(_ bundleId: String, _ completionHandler: @escaping (_ error: Error?, _ tunnel: NETunnelProviderManager?) -> Void) {
        VPN_BUNDLE_ID = bundleId;
        
        TunnelManager.instance.setTunnel {
            switch TunnelManager.instance.tunnel {
            case .failure(let error):
                return completionHandler(error, nil)
            case .success(let tunnel):
                return completionHandler(nil, tunnel)
            case .none:
                fatalError("IMPOSSIBLE: Attempted to set the VPN tunnel, but didn't get an error nor a tunnel.")
            }
            
        };
    }
    
    static func withTunnel(_ f: (_ tunnel: NETunnelProviderManager) -> Any) -> Any? {
        assert(TunnelManager.instance.tunnel != nil)
        
        switch TunnelManager.instance.tunnel {
        case .success(let tunnel):
            return f(tunnel)
        case .failure, .none:
            Logger.global?.log(message: "Attempted to use the VPN tunnel, but it's not available.")
            return nil
        }
    }
    
    static func checkStatus(_ callback: @escaping (String, String, String) -> Void) {
        _ = TunnelManager.withTunnel { tunnel in
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

            guard let session = tunnel.connection as? NETunnelProviderSession
            else {
                callback("", "", "")
                return
            }

            return TunnelMessage.getRuntimeConfiguration.send() { [callback] data in                
                guard let data = data,
                    let configString = String(data: data, encoding: .utf8)
                else {
                    Logger.global?.log(message: "Failed to convert data to string")
                    callback("", "", "")
                    return
                }

                callback("\(serverIpv4Gateway!)", "\(deviceIpv4Address!)", configString)
            }
        }
    }

    private func setTunnel(_ completionHandler: @escaping () -> Void) {
        NETunnelProviderManager.loadAllFromPreferences { [weak self] managers, error in
            if self == nil {
                Logger.global?.log(message: "We are shutting down.")
                return
            }

            if let error = error {
                Logger.global?.log(message: "Loading from preference failed: \(error)")
                return
            }

            let nsManagers = managers ?? []
            Logger.global?.log(message: "We have received \(nsManagers.count) managers.")

            let tunnel = nsManagers.first(where: TunnelManager.isOurManager(_:))
            if tunnel == nil {
                Logger.global?.log(message: "Creating the tunnel")
                self!.tunnel = .success(NETunnelProviderManager())
                return
            }

            Logger.global?.log(message: "Tunnel already exists")
            self!.tunnel = .success(tunnel!)

            completionHandler();
        }
    }

    private static func isOurManager(_ manager: NETunnelProviderManager) -> Bool {
        guard
            let proto = manager.protocolConfiguration,
            let tunnelProto = proto as? NETunnelProviderProtocol
        else {
            Logger.global?.log(message: "Ignoring manager because the proto is invalid.")
            return false
        }

        if (tunnelProto.providerBundleIdentifier == nil) {
            Logger.global?.log(message: "Ignoring manager because the bundle identifier is null.")
            return false
        }

        if (tunnelProto.providerBundleIdentifier != VPN_BUNDLE_ID) {
            Logger.global?.log(message: "Ignoring manager because the bundle identifier doesn't match.")
            return false;
        }

        Logger.global?.log(message: "Found the manager with the correct bundle identifier: \(tunnelProto.providerBundleIdentifier!)")
        return true
    }
}
